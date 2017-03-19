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

#ifndef INCLUDED_CCSDS_INSERT_IDLE_FRAME_IMPL_H
#define INCLUDED_CCSDS_INSERT_IDLE_FRAME_IMPL_H

#include <ccsds/insert_idle_frame.h>

namespace gr {
  namespace ccsds {

    class insert_idle_frame_impl : public insert_idle_frame
    {
     private:
      // Nothing to declare in this block.
      std::vector<gr_complex> d_symbols;
      uint16_t d_frame_size;
      uint32_t d_num_fillframes_added;
      bool d_started;

     public:
      insert_idle_frame_impl(const std::vector<gr_complex> &modulated_vector);
      ~insert_idle_frame_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);

      uint32_t num_fillframes_added() const {return d_num_fillframes_added;}
    };

  } // namespace ccsds
} // namespace gr

#endif /* INCLUDED_CCSDS_INSERT_IDLE_FRAME_IMPL_H */
