/*!
 * \file gps_l1_ca_pcps_acquisition.cc
 * \brief Brief description of the file here
 * \author Javier Arribas, 2011. jarribas(at)cttc.es
 *         Luis Esteve, 2011. luis(at)epsilon-formacion.com
 *
 * Detailed description of the file here if needed.
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

#include "gps_l1_ca_pcps_acquisition.h"

#include "configuration_interface.h"
#include <iostream>
#include <gnuradio/gr_io_signature.h>
#include <gnuradio/gr_stream_to_vector.h>
#include <gnuradio/gr_vector_to_stream.h>
#include <gnuradio/gr_complex_to_interleaved_short.h>
#include <gnuradio/gr_interleaved_short_to_complex.h>

#include <glog/log_severity.h>
#include <glog/logging.h>

using google::LogMessage;

GpsL1CaPcpsAcquisition::GpsL1CaPcpsAcquisition(
        ConfigurationInterface* configuration, std::string role,
        unsigned int in_streams, unsigned int out_streams,
        gr_msg_queue_sptr queue) :
    role_(role), in_streams_(in_streams), out_streams_(out_streams), queue_(
            queue)
{

    std::string default_item_type = "gr_complex";
    std::string default_dump_filename = "./data/acquisition.dat";

    DLOG(INFO) << "role " << role;

    item_type_ = configuration->property(role + ".item_type",
            default_item_type);

    satellite_ = 0;
    fs_in_ = configuration->property(role + ".fs_in", 2048000);
    if_ = configuration->property(role + ".ifreq", 0);
    dump_ = configuration->property(role + ".dump", false);
    shift_resolution_ = configuration->property(role + ".doppler_max", 15);
    sampled_ms_ = configuration->property(role + ".sampled_ms", 1);
    dump_filename_ = configuration->property(role + ".dump_filename",
            default_dump_filename);

    //--- Find number of samples per spreading code ----------------------------
    const signed int _codeFreqBasis = 1023000; //Hz
    const signed int _codeLength = 1023;
    std::cout << fs_in_ << " " << role << std::endl;
    vector_length_ = round(fs_in_ / (_codeFreqBasis / _codeLength));

    printf("vector_length_ %i\n\r", vector_length_);

    if (item_type_.compare("gr_complex") == 0)
    {
        item_size_ = sizeof(gr_complex);
        acquisition_cc_ = gps_l1_ca_pcps_make_acquisition_cc(sampled_ms_,
                shift_resolution_, if_, fs_in_, vector_length_, queue_,
                dump_, dump_filename_);
        stream_to_vector_ = gr_make_stream_to_vector(item_size_,
                vector_length_);
    }
    else
    {
        LOG_AT_LEVEL(WARNING) << item_type_
                << " unknown acquisition item type";
    }

    DLOG(INFO) << "stream_to_vector(" << stream_to_vector_->unique_id()
            << ")";
}

GpsL1CaPcpsAcquisition::~GpsL1CaPcpsAcquisition()
{
}

void GpsL1CaPcpsAcquisition::set_satellite(unsigned int satellite)
{
    satellite_ = satellite;

    if (item_type_.compare("gr_complex") == 0)
    {
        acquisition_cc_->set_satellite(satellite_);
    }
}

void GpsL1CaPcpsAcquisition::set_channel(unsigned int channel)
{
    channel_ = channel;

    if (item_type_.compare("gr_complex") == 0)
    {
        acquisition_cc_->set_channel(channel_);
    }
}

void GpsL1CaPcpsAcquisition::set_threshold(float threshold)
{
    threshold_ = threshold;

    if (item_type_.compare("gr_complex") == 0)
    {
        acquisition_cc_->set_threshold(threshold_);
    }
}

void GpsL1CaPcpsAcquisition::set_doppler_max(unsigned int doppler_max)
{
    doppler_max_ = doppler_max;

    if (item_type_.compare("gr_complex") == 0)
    {
        acquisition_cc_->set_doppler_max(doppler_max_);
    }

}

void GpsL1CaPcpsAcquisition::set_doppler_step(unsigned int doppler_step)
{
    doppler_step_ = doppler_step;

    if (item_type_.compare("gr_complex") == 0)
    {
        acquisition_cc_->set_doppler_step(doppler_step_);
    }

}

void GpsL1CaPcpsAcquisition::set_channel_queue(
        concurrent_queue<int> *channel_internal_queue)
{
    channel_internal_queue_ = channel_internal_queue;

    if (item_type_.compare("gr_complex") == 0)
    {
        acquisition_cc_->set_channel_queue(channel_internal_queue_);
    }
}

signed int GpsL1CaPcpsAcquisition::prn_code_phase()
{

    if (item_type_.compare("gr_complex") == 0)
    {
        return acquisition_cc_->prn_code_phase();
    }
    else
    {
        return 0;
    }
}

unsigned long int GpsL1CaPcpsAcquisition::get_sample_stamp()
{
    if (item_type_.compare("gr_complex") == 0)
    {
        return acquisition_cc_->get_sample_stamp();
    }
    else
    {
        return 0;
    }
}
float GpsL1CaPcpsAcquisition::doppler_freq_shift()
{

    if (item_type_.compare("gr_complex") == 0)
    {
        return acquisition_cc_->doppler_freq();
    }
    else
    {
        return 0;
    }
}

signed int GpsL1CaPcpsAcquisition::mag()
{
    if (item_type_.compare("gr_complex") == 0)
    {
        return acquisition_cc_->mag();
    }
    else
    {
        return 0;
    }
}

void GpsL1CaPcpsAcquisition::reset()
{

    if (item_type_.compare("gr_complex") == 0)
    {
        acquisition_cc_->set_active(true);
    }
}

void GpsL1CaPcpsAcquisition::connect(gr_top_block_sptr top_block)
{

    if (item_type_.compare("gr_complex") == 0)
    {
        top_block->connect(stream_to_vector_, 0, acquisition_cc_, 0);
    }

}

void GpsL1CaPcpsAcquisition::disconnect(gr_top_block_sptr top_block)
{

    if (item_type_.compare("gr_complex") == 0)
    {
        top_block->disconnect(stream_to_vector_, 0, acquisition_cc_, 0);
    }
}

gr_basic_block_sptr GpsL1CaPcpsAcquisition::get_left_block()
{
    return stream_to_vector_;
}

gr_basic_block_sptr GpsL1CaPcpsAcquisition::get_right_block()
{
    return acquisition_cc_;
}
