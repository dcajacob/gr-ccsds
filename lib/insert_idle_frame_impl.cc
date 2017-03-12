/* -*- c++ -*- */
/*
 * Copyright 2017 <+YOU OR YOUR COMPANY+>.
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
#include "insert_idle_frame_impl.h"

namespace gr {
  namespace ccsds {

    insert_idle_frame::sptr
    insert_idle_frame::make(const std::vector<gr_complex> &modulated_vector)
    {
      return gnuradio::get_initial_sptr
        (new insert_idle_frame_impl(modulated_vector));
    }

    /*
     * The private constructor
     */
    insert_idle_frame_impl::insert_idle_frame_impl(const std::vector<gr_complex> &modulated_vector)
      : gr::block("insert_idle_frame",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
      d_frame_size(modulated_vector.size()),
      d_num_fillframes_added(0)
    {
      set_output_multiple(d_frame_size);
      d_symbols = modulated_vector;
    }

    /*
     * Our virtual destructor.
     */
    insert_idle_frame_impl::~insert_idle_frame_impl()
    {
    }

    void
    insert_idle_frame_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int
    insert_idle_frame_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      if ((ninput_items[0] >= d_frame_size) && (d_frame_size >= noutput_items)) {

        uint8_t num_frames = ninput_items[0] / d_frame_size;
        num_frames = num_frames*d_frame_size / noutput_items;

        printf("\nProcessing %d items in %d frames from input.\n\n", num_frames*d_frame_size, num_frames);
        memcpy(out, &in[0], sizeof(gr_complex)*num_frames*d_frame_size);
        consume_each (num_frames*d_frame_size);

        return (num_frames*d_frame_size);
      } else if (d_frame_size >= noutput_items) {
        // Push out a single idle frame
        //printf("%d input items available. %d output items available.\n", ninput_items[0], noutput_items);
        //printf("Pushing an idle packet of size %d.\n", d_frame_size);
        for(size_t i = 0; i < d_frame_size; i++) {
          out[i] = d_symbols[i];
        }
        //consume_each (d_frame_size);

        d_num_fillframes_added++;

        return d_frame_size;
      } else {
        return 0;
      }
    }

  } /* namespace ccsds */
} /* namespace gr */
