/* -*- c++ -*- */
/*
 * Copyright 2013 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "constellation_soft_decoder_cf_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
  namespace digital {

    constellation_soft_decoder_cf::sptr
    constellation_soft_decoder_cf::make(constellation_sptr constellation)
    {
      return gnuradio::get_initial_sptr
	(new constellation_soft_decoder_cf_impl(constellation));
    }

    constellation_soft_decoder_cf_impl::
    constellation_soft_decoder_cf_impl(constellation_sptr constellation)
      : sync_interpolator("constellation_soft_decoder_cf",
                          io_signature::make(1, 1, sizeof(gr_complex)),
                          io_signature::make(1, 1, sizeof(float)),
                          constellation->bits_per_symbol()),
	d_constellation(constellation),
	d_dim(constellation->dimensionality()),
        d_bps(constellation->bits_per_symbol())
    {
      d_rotation_index = 0;

      message_port_register_in(pmt::mp("rotate_constellation"));
      set_msg_handler(
        pmt::mp("rotate_constellation"),
        boost::bind(&constellation_soft_decoder_cf_impl::handle_rotate_constellation,
                    this, _1));
    }

    constellation_soft_decoder_cf_impl::~constellation_soft_decoder_cf_impl()
    {
    }

    void
    constellation_soft_decoder_cf_impl::handle_rotate_constellation(pmt::pmt_t constellation_pmt)
    {
      /*if(pmt::is_any(constellation_pmt)) {
        boost::any constellation_any = pmt::any_ref(constellation_pmt);
        constellation_sptr constellation = boost::any_cast<constellation_sptr>(
          constellation_any);
        set_constellation(constellation);
      }*/
      set_constellation();
    }

    void
    constellation_soft_decoder_cf_impl::rotate_constellation()
    {
      /*
        Rotate the constellation on each call according to the
        constellation's dihedral symmetry.
      */

      // rotate the constellation vector by one element
      if (d_rotation_index < d_constellation->rotational_symmetry()) {}
        std::rotate(d_constellation->constellation().begin(),
                    d_constellation->constellation().end()-1,
                    d_constellation->constellation().end());
      } else {
        // TODO: Swap first and second half of vector
        // rotate the constellation vector by one element

        std::rotate(d_constellation->constellation().begin(),
                    d_constellation->constellation().end()-1,
                    d_constellation->constellation().end());
      }

      d_rotation_index = (d_rotation_index + 1) % d_constellation->rotational_symmetry();

      if (d_rotation_index == 0) {
        // TODO: flip back to normal
        //  d_constellation =
      }
    }

    int
    constellation_soft_decoder_cf_impl::work(int noutput_items,
                                             gr_vector_const_void_star &input_items,
                                             gr_vector_void_star &output_items)
    {
      gr_complex const *in = (const gr_complex*)input_items[0];
      float *out = (float*)output_items[0];

      std::vector<float> bits;

      // FIXME: figure out how to manage d_dim
      for(int i = 0; i < noutput_items/d_bps; i++) {
	bits = d_constellation->soft_decision_maker(in[i]);
        for(size_t j = 0; j < bits.size(); j++) {
          out[d_bps*i+j] = bits[j];
        }
      }

      return noutput_items;
    }

  } /* namespace digital */
} /* namespace gr */
