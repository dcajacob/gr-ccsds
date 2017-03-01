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

#ifndef INCLUDED_DIGITAL_CONSTELLATION_SOFT_DECODER_CF_IMPL_H
#define INCLUDED_DIGITAL_CONSTELLATION_SOFT_DECODER_CF_IMPL_H

#include <gnuradio/digital/constellation_soft_decoder_cf.h>

namespace gr {
  namespace digital {

    class constellation_soft_decoder_cf_impl : public constellation_soft_decoder_cf
    {
    private:
      constellation_sptr d_constellation;
      unsigned int d_dim;
      int d_bps;
      uint8_t d_rotation_index;

      /*!
       * Message handler port to receiver a new constellation.
       * constellation_pmt is a pmt_any; constellation objects have
       * an as_pmt function that can be used for this purpose.
       */
      void handle_rotate_constellation(pmt::pmt_t constellation_pmt);

      //! Set the constellation used.
      //! Typically used when we receive a tag with a value for this.
      void rotate_constellation(constellation_sptr constellation);

    public:
      constellation_soft_decoder_cf_impl(constellation_sptr constellation);
      ~constellation_soft_decoder_cf_impl();

      int work(int noutput_items,
               gr_vector_const_void_star &input_items,
               gr_vector_void_star &output_items);
    };

  } /* namespace digital */
} /* namespace gr */

#endif /* INCLUDED_DIGITAL_CONSTELLATION_SOFT_DECODER_CF_IMPL_H */
