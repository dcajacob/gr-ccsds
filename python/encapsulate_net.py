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

from time import sleep
import struct

import numpy
from gnuradio import gr
import collections
import pmt
import array

debug = True

class encapsulate_net(gr.basic_block):
    """
    docstring for block encapsulate_net
    """
    def __init__(self, mtu=1115, pad_byte=0x7E, aggregate=False, timeout=100):
        gr.basic_block.__init__(self,
            name="encapsulate_net",
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

        if pmt.length(msg) + 2 > self.mtu:
            print "[ERROR] Transmitted frame is too long (%d bytes).  Cannot exceed %d bytes." % (pmt.length(msg), self.mtu - 2)
            return

        #length = struct.pack('h', pmt.length(msg))
        #print length, pmt.length(msg)

        buff = list()
        #buff.extend([x for x in length])
        #buff.extend(pmt.u8vector_elements(msg))
        length = pmt.length(msg)
        buff.append(length >> 8) # MSB
        buff.append(length & 0xFF) # LSB
        buff.extend(pmt.u8vector_elements(msg))

        pad_length = self.mtu - len(buff)
        if pad_length:
            buff.extend([self.pad_byte] * pad_length)

        if debug:
            # FIXME: This printing out a value 1 byte short of what it should be...
            print "Pushing a packet of length %d bytes" % length

        self.message_port_pub(pmt.intern('out'), pmt.cons(pmt.PMT_NIL, pmt.init_u8vector(len(buff), buff)))
