/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#include <rtp/api/units/data_rate.h>

#include <rtp/api/array_view.h>
#include <rtp/base/strings/string_builder.h>

namespace qpidnetwork {

std::string ToString(DataRate value) {
	char buf[64];
	SimpleStringBuilder sb(buf);
	if (value.IsPlusInfinity()) {
		sb << "+inf bps";
	} else if (value.IsMinusInfinity()) {
		sb << "-inf bps";
	} else {
		if (value.bps() == 0 || value.bps() % 1000 != 0) {
			sb << value.bps() << " bps";
		} else {
			sb << value.kbps() << " kbps";
		}
	}
	return sb.str();
}
}  // namespace qpidnetwork
