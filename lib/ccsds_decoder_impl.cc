/* -*- c++ -*- */
/*
 * Copyright 2016 André Løfaldli.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include <volk/volk.h>
#include <gnuradio/digital/constellation.h>
#include "ccsds_decoder_impl.h"
#include "ccsds.h"
#include "reed_solomon.h"

#define STATE_SYNC_SEARCH 0
#define STATE_CODEWORD 1

namespace gr {
  namespace ccsds {

    ccsds_decoder::sptr
    ccsds_decoder::make(int threshold, bool rs_decode, bool deinterleave, bool descramble, bool verbose, bool printing, std::vector<gr::digital::constellation_sptr> constellations)
    {
      return gnuradio::get_initial_sptr
        (new ccsds_decoder_impl(threshold, rs_decode, deinterleave, descramble, verbose, printing, constellations));
    }

    ccsds_decoder_impl::ccsds_decoder_impl(int threshold, bool rs_decode, bool deinterleave, bool descramble, bool verbose, bool printing, std::vector<gr::digital::constellation_sptr> constellations)
      : gr::sync_block("ccsds_decoder",
              gr::io_signature::make(1, 1, sizeof(uint8_t)),
              gr::io_signature::make(0, 0, 0)),
        d_threshold(threshold),
        d_rs_decode(rs_decode),
        d_deinterleave(deinterleave),
        d_descramble(descramble),
        d_verbose(verbose),
        d_printing(printing),
        d_num_frames_failed(0),
        d_num_frames_received(0),
        d_num_frames_decoded(0),
        d_num_subframes_decoded(0),
        d_constellations(constellations)
    {
      message_port_register_out(pmt::mp("out"));
      message_port_register_out(pmt::mp("constellation"));

      for (uint8_t i=0; i<SYNC_WORD_LEN; i++) {
          d_sync_word = (d_sync_word << 8) | (SYNC_WORD[i] & 0xff);
      }

      d_hysteresis = 0;
      d_constellation = constellations[0];
      d_alt_constel_1 = constellations[1];
      d_alt_constel_2 = constellations[2];
      d_alt_constel_3 = constellations[3];
      //gr::digital::constellation d_constellation;
      //(gr_complex(-1, -1), gr_complex(1, -1), gr_complex(-1, 1), gr_complex(1, 1)), {0, 2, 1, 3}, 4, 1);
      //(gr_complex(-1, -1), gr_complex(1, -1), gr_complex(-1, 1), gr_complex(1, 1)), (0, 2, 1, 3), 4, 1)
      //gr::digital::constellation d_alt_constel_1 ((gr_complex(-1, -1), gr_complex(1, -1), gr_complex(-1, 1), gr_complex(1, 1)), (1, 3, 0, 2), 4, 1);
      //gr::digital::constellation d_alt_constel_2 ((gr_complex(-1, -1), gr_complex(1, -1), gr_complex(-1, 1), gr_complex(1, 1)), (2, 0, 3, 1), 4, 1);
      //gr::digital::constellation d_alt_constel_3 ((gr_complex(-1, -1), gr_complex(1, -1), gr_complex(-1, 1), gr_complex(1, 1)), (3, 1, 2, 0), 4, 1);
      d_alt_constel_index = 0;

      /*
      // Create an alternative sync word that corresponds to a 90 deg
      //  constellation rotation. Others covered by differential encoding.
      //d_alt_sync_word = reverse_and_invert(d_sync_word, 2, 0x02);
      d_alt_sync_word1 = sync_word_munge(1, d_sync_word, 2, 0x02, 32);
      d_alt_sync_word2 = sync_word_munge(2, d_sync_word, 2, 0x02, 32);
      d_alt_sync_word3 = sync_word_munge(3, d_sync_word, 2, 0x02, 32);
      d_alt_sync_word4 = sync_word_munge(4, d_sync_word, 2, 0x02, 32);
      d_alt_sync_word5 = sync_word_munge(5, d_sync_word, 2, 0x02, 32);
      if (d_verbose) {
          printf("\tNormal sync word:\t%Zd\n", static_cast<uint64_t>(d_sync_word));
          printf("\tFormed an alternate sync word:\t%zd\n", static_cast<uint64_t>(d_alt_sync_word1));
          printf("\tFormed an alternate sync word:\t%zd\n", static_cast<uint64_t>(d_alt_sync_word2));
          printf("\tFormed an alternate sync word:\t%zd\n", static_cast<uint64_t>(d_alt_sync_word3));
          printf("\tFormed an alternate sync word:\t%zd\n", static_cast<uint64_t>(d_alt_sync_word4));
          printf("\tFormed an alternate sync word:\t%zd\n", static_cast<uint64_t>(d_alt_sync_word5));
      }
      d_alt_sync_state = 0;
      */

      enter_sync_search();
    }

    ccsds_decoder_impl::~ccsds_decoder_impl()
    {
    }

    int
    ccsds_decoder_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const uint8_t *in = (const uint8_t *) input_items[0];

      uint16_t count = 0;
      while (count < noutput_items) {
          switch (d_decoder_state) {
              case STATE_SYNC_SEARCH:
                  // get next bit
                  d_data_reg = (d_data_reg << 1) | (in[count++] & 0x01);
                  if (compare_sync_word()) {
                      if (d_verbose) printf("\tsync word detected\n");
                      d_num_frames_failed = 0; // reset the nodetect counter
                      d_num_frames_received++;
                      d_alt_sync_state = 0;
                      enter_codeword();
                      break;
                  /*
                  } else if (compare_alt_sync_word(d_alt_sync_word1)) {
                      if (d_verbose) printf("\talternate sync word detected %zd\t1\t-Q I\n", static_cast<uint64_t>(sync_word_munge(1, d_data_reg, 2, 0x02, 32)));
                      d_num_frames_received++;
                      d_alt_sync_state = 1;
                      enter_codeword();
                      break;
                  } else if (compare_alt_sync_word(d_alt_sync_word2)) {
                      if (d_verbose) printf("\talternate sync word detected %zd\t2\tQ I\n", static_cast<uint64_t>(sync_word_munge(2, d_data_reg, 2, 0x02, 32)));
                      d_num_frames_received++;
                      d_alt_sync_state = 2;
                      enter_codeword();
                      break;
                  } else if (compare_alt_sync_word(d_alt_sync_word3)) {
                      if (d_verbose) printf("\talternate sync word detected %zd\t3\tQ -I\n", static_cast<uint64_t>(sync_word_munge(3, d_data_reg, 2, 0x02, 32)));
                      d_num_frames_received++;
                      d_alt_sync_state = 3;
                      enter_codeword();
                      break;
                  } else if (compare_alt_sync_word(d_alt_sync_word4)) {
                      if (d_verbose) printf("\talternate sync word detected %zd\t4\t-I Q\n", static_cast<uint64_t>(sync_word_munge(4, d_data_reg, 2, 0x02, 32)));
                      d_num_frames_received++;
                      d_alt_sync_state = 4;
                      enter_codeword();
                      break;
                  } else if (compare_alt_sync_word(d_alt_sync_word5)) {
                      if (d_verbose) printf("\talternate sync word detected %zd\t5\tI -Q\n", static_cast<uint64_t>(sync_word_munge(5, d_data_reg, 2, 0x02, 32)));
                      d_num_frames_received++;
                      d_alt_sync_state = 5;
                      enter_codeword();
                      break;
                  */
                  } else {
                      d_num_frames_failed++;
                  }
                  break;
              case STATE_CODEWORD:
                  // get next bit and pack then into full bytes
                  d_data_reg = (d_data_reg << 1) | (in[count++] & 0x01);
                  d_bit_counter++;
                  if (d_bit_counter == 8) {

                    /*
                    switch (d_alt_sync_state) {
                        case 0:
                            break;
                        case 1:
                            d_data_reg = sync_word_munge(1, d_data_reg, 2, 0x02, 8) & 0xFF;
                            break;
                        case 2:
                            d_data_reg = sync_word_munge(2, d_data_reg, 2, 0x02, 8) & 0xFF;
                            break;
                        case 3:
                            d_data_reg = sync_word_munge(3, d_data_reg, 2, 0x02, 8) & 0xFF;
                            break;
                        case 4:
                            d_data_reg = sync_word_munge(4, d_data_reg, 2, 0x02, 8) & 0xFF;
                            break;
                        case 5:
                            d_data_reg = sync_word_munge(5, d_data_reg, 2, 0x02, 8) & 0xFF;
                      }
                      */

                      d_codeword[d_byte_counter] = d_data_reg;
                      d_byte_counter++;
                      d_bit_counter = 0;
                  }
                  // once the full codeword is loaded, try to decode the packet
                  if (d_byte_counter == CODEWORD_LEN) {
                      if (d_verbose) printf("\tloaded codeword of length %i\n", CODEWORD_LEN);
                      if (d_printing) print_bytes(d_codeword, CODEWORD_LEN);

                      bool success = decode_frame();
                      if (success) {
                          pmt::pmt_t pdu(pmt::cons(pmt::PMT_NIL, pmt::make_blob(d_payload, DATA_LEN)));
                          message_port_pub(pmt::mp("out"), pdu);
                      } else {
                          if ((d_num_frames_failed > 1000000) || ((d_num_frames_received > 100) && (d_num_frames_decoded < 50)) && (d_hysteresis == 0)) {
                              gr::digital::constellation_sptr constel;
                              d_alt_constel_index = (d_alt_constel_index + 1) % 4;
                              constel = d_constellations[d_alt_constel_index];
                              /*
                              switch(d_alt_constel_index) {
                                  case 0:
                                      constel = d_constellation;
                                      break;
                                  case 1:
                                      constel = d_alt_constel_1;
                                      break;
                                  case 2:
                                      constel = d_alt_constel_2;
                                      break;
                                  case 3:
                                      constel = d_alt_constel_3;
                              }
                              */

                              //boost::any constellation_any = pmt::any_ref(constellation_pmt);
                              //constellation_sptr constellation = boost::any_cast<constellation_sptr>(
                              //  constellation_any);

                              pmt::pmt_t pdu(pmt::cons(pmt::PMT_NIL, pmt::make_any(constel)));
                              message_port_pub(pmt::mp("constellation"), pdu);

                              d_hysteresis = 1000;
                          } else {
                              d_hysteresis--;
                          }
                      }

                      if (d_verbose) {
                          printf("\tframes received: %i\n\tframes decoded: %i\n\tsubframes decoded: %i\n",
                                  d_num_frames_received,
                                  d_num_frames_decoded,
                                  d_num_subframes_decoded);
                      }
                      enter_sync_search();
                  }
                  break;
          }
      }
      return noutput_items;
    }

    void
    ccsds_decoder_impl::enter_sync_search()
    {
        if (d_verbose) printf("enter sync search\n");
        d_decoder_state = STATE_SYNC_SEARCH;
        d_data_reg = 0;
    }
    void
    ccsds_decoder_impl::enter_codeword()
    {
        if (d_verbose) printf("enter codeword\n");
        d_decoder_state = STATE_CODEWORD;
        d_byte_counter = 0;
        d_bit_counter = 0;
    }
    bool ccsds_decoder_impl::compare_sync_word()
    {
        uint32_t nwrong = 0;
        uint32_t wrong_bits = d_data_reg ^ d_sync_word;
        volk_32u_popcnt(&nwrong, wrong_bits);
        return nwrong <= d_threshold;
    }

    bool ccsds_decoder_impl::compare_alt_sync_word(uint32_t alt_sync_word)
    {
        uint32_t nwrong = 0;
        uint32_t wrong_bits = d_data_reg ^ alt_sync_word;
        volk_32u_popcnt(&nwrong, wrong_bits);
        return nwrong <= d_threshold;
    }

    bool ccsds_decoder_impl::decode_frame()
    {
        // this will be set to false if a codeword is not decodable
        bool success = true;

        if (d_descramble) {
            descramble(d_codeword, CODEWORD_LEN);
        }

        // deinterleave and decode rs blocks
        uint8_t rs_block[RS_BLOCK_LEN];
        int8_t nerrors;
        for (uint8_t i=0; i<RS_NBLOCKS; i++) {
            for (uint8_t j=0; j<RS_BLOCK_LEN; j++) {

                if (d_deinterleave) {
                    rs_block[j] = d_codeword[i+(j*RS_NBLOCKS)];
                } else {
                    rs_block[j] = d_codeword[i*RS_BLOCK_LEN + j];
                }

            }
            if (d_rs_decode) {
                nerrors = d_rs.decode(rs_block);
                if (nerrors == -1) {
                    if (d_verbose) printf("\tcould not decode rs block #%i\n", i);
                    success = false;
                } else {
                    if (d_verbose) printf("\tdecoded rs block #%i with %i errors\n", i, nerrors);
                    d_num_subframes_decoded++;
                }
            }
            memcpy(&d_payload[i*RS_DATA_LEN], rs_block, RS_DATA_LEN);
        }

        if (success) d_num_frames_decoded++;

        return success;
    }

    uint8_t ccsds_decoder_impl::reverse(uint8_t x, uint8_t n)
    {
        // Bit-reverse every n bits
        uint8_t result = 0;
        for (uint8_t i=0; i<n; i++) {
            if ((x >> i) & 1)
                result |= 1 << (n - 1 - i);
        }

        return result;
    }

    uint8_t ccsds_decoder_impl::reverse_dibit(uint32_t x, uint8_t length, uint8_t i)
    {
        uint32_t result = 0;
        uint8_t sym = 0;
        const int n = 2;

        return reverse((x >> ((length-n) - n*i)) & 0x03, n);  // FIXME: replace 0x03 with 2^n-1?
    }

    uint8_t ccsds_decoder_impl::invert(uint8_t x, uint8_t mask)
    {
        // Invert the masked bits
        return x ^ mask;
    }

    uint8_t ccsds_decoder_impl::invert_dibit(uint8_t x, uint8_t length, uint8_t mask, uint8_t i)
    {
        const int n = 2;

        return invert((x >> ((length-n) - n*i)) & 0x03, mask);  // FIXME: replace 0x03 with 2^n-1?
    }

    uint32_t ccsds_decoder_impl::sync_word_munge(uint8_t flavor, uint32_t x, uint8_t n, uint8_t mask, uint8_t length)
    {
        uint32_t temp = 0;
        uint32_t result = 0;
        uint8_t sym = 0;

        for (uint8_t i=0; i<(length/n); i++) {
            switch (flavor) {
                case 1:
                    //sym = invert(reverse((x >> ((length-n) - n*i)) & 0x3, n), 0x02); // -Q I
                    sym = invert(reverse_dibit(x, length, i), 0x02); // -Q I
                    break;
                case 2:
                    //sym = reverse((x >> ((length-n) - n*i)) & 0x3, n); // Q I
                    sym = reverse_dibit(x, length, i); // Q I
                    break;
                case 3:
                    //sym = invert((x >> ((length-n) - n*i)) & 0x3, 0x02); // Q -I
                    sym = invert(reverse_dibit(x, length, i), 0x01); // Q -I
                    break;
                case 4:
                    sym = invert_dibit(x, length, 0x02, i); // -I Q
                    break;
                case 5:
                    sym = invert_dibit(x, length, 0x01, i); // I -Q
            }

            //sym = reverse((x >> (30 - 2*i)) & 0x3, 2);
            //sym = invert(reverse((x >> (30 - 2*i)) & 0x3, 2), 0x01);
            temp = (temp << n) | (sym & 0xFF);
        }
        result = temp;

        return result;
    }
  } /* namespace ccsds */
} /* namespace gr */
