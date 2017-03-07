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


#ifndef INCLUDED_CCSDS_INSERT_IDLE_FRAME_H
#define INCLUDED_CCSDS_INSERT_IDLE_FRAME_H

#include <ccsds/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace ccsds {

    /*!
     * \brief <+description of block+>
     * \ingroup ccsds
     *
     */
    class CCSDS_API insert_idle_frame : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<insert_idle_frame> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of ccsds::insert_idle_frame.
       *
       * To avoid accidental use of raw pointers, ccsds::insert_idle_frame's
       * constructor is in a private implementation
       * class. ccsds::insert_idle_frame::make is the public interface for
       * creating new instances.
       */
      static sptr make(const std::vector<gr_complex> &modulated_vector);

      /*!
       * \brief return number of fill frames added
       */
      virtual uint32_t num_fillframes_added() const = 0;
    };

  } // namespace ccsds
} // namespace gr

#endif /* INCLUDED_CCSDS_INSERT_IDLE_FRAME_H */
