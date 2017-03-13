#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2017 <+YOU OR YOUR COMPANY+>.
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

import struct

import numpy
from gnuradio import gr
import collections
import pmt
import array

debug = True

class decapsulate_net(gr.basic_block):
    """
    docstring for block decapsulate_net
    """
    def __init__(self, mtu=1115, pad_byte=0x7E, disaggregate=False):
        gr.basic_block.__init__(self,
            name="decapsulate_net",
            in_sig=None,
            out_sig=None)

        self.mtu = mtu
        self.pad_byte = pad_byte

        self.message_port_register_in(pmt.intern('in'))
        self.set_msg_handler(pmt.intern('in'), self.handle_msg)
        self.message_port_register_out(pmt.intern('out'))

    def handle_msg(self, msg_pmt):
        msg = pmt.cdr(msg_pmt)
        if not pmt.is_u8vector(msg):
            print "[ERROR] Received invalid message type. Expected u8vector"
            return

        if pmt.length(msg) > self.mtu:
            print "[ERROR] Received frame is too long (%d bytes).  Cannot exceed %d bytes." % (pmt.length(msg), self.mtu)
            return

        #print pmt.u8vector_elements(msg)[0:2]
        #length = struct.unpack('h', pmt.u8vector_elements(msg)[0:2])
        length_bytes = pmt.u8vector_elements(msg)[0:2]

        # Sum the first 2 bytes to get packet length
        #length = length_bytes[0] << 8 | length_bytes[1]
        length = sum([x << (8 * (len(length_bytes) - idx - 1)) for idx, x in enumerate(length_bytes)])
        #print length, pmt.length(msg)

        if length == 0: # caught a Filler packet, drop it
            return

        if length > (self.mtu - 2):
            print "[ERROR] Received frame reported length is too long (%d bytes).  Cannot exceed %d bytes." % (length, (self.mtu - 2))
            if debug:
                print pmt.u8vector_elements(msg)[0:8]
            return

        buff = list()
        buff.extend(pmt.u8vector_elements(msg)[2:2+length])
        #print buff

        # TODO: Check for pad byte if there are any

        #print pmt.length(buff)
        buff = array.array('B', buff)

        if debug:
            print "Received a packet of length %d bytes" % length

        self.message_port_pub(pmt.intern('out'), pmt.cons(pmt.PMT_NIL, pmt.init_u8vector(len(buff), buff)))
