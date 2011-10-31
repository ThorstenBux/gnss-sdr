/*!
 * \file null_sink_output_filter.h
 * \brief Brief description of the file here
 * \author Carlos Aviles, 2010. carlos.avilesr(at)googlemail.com
 *
 * This class represents an implementation of an output filter that
 * sends its input to a null sink.
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2011  (see AUTHORS file for a list of contributors)
 *
 * GNSS-SDR is a software defined Global Navigation
 *          Satellite Systems receiver
 *
 * This file is part of GNSS-SDR.
 *
 * GNSS-SDR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * at your option) any later version.
 *
 * GNSS-SDR is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNSS-SDR. If not, see <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------
 */



#ifndef NULL_SINK_OUTPUT_FILTER_H_
#define NULL_SINK_OUTPUT_FILTER_H_

#include "gnss_block_interface.h"

#include <gr_null_sink.h>

class ConfigurationInterface;

class NullSinkOutputFilter : public GNSSBlockInterface
{

public:
    NullSinkOutputFilter(ConfigurationInterface* configuration,
            std::string role,
            unsigned int in_streams,
            unsigned int out_streams);

    virtual ~NullSinkOutputFilter();

    std::string item_type()
    {
        return item_type_;
    }
    std::string role()
    {
        return role_;
    }
    std::string implementation()
    {
        return "NullSinkOutputFilter";
    }
    size_t item_size()
    {
        return item_size_;
    }

    void connect(gr_top_block_sptr top_block);
    void disconnect(gr_top_block_sptr top_block);
    gr_basic_block_sptr get_left_block();
    gr_basic_block_sptr get_right_block();

private:

    gr_block_sptr sink_;
    size_t item_size_;

    std::string item_type_;
    std::string role_;

    unsigned int in_streams_;
    unsigned int out_streams_;
};

#endif /*NULL_SINK_OUTPUT_FILTER_H_*/