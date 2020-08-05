/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#include <rtp/api/units/frequency.h>
#include <rtp/base/strings/string_builder.h>

namespace mediaserver {
std::string ToString(Frequency value) {
	char buf[64];
	SimpleStringBuilder sb(buf);
	if (value.IsPlusInfinity()) {
		sb << "+inf Hz";
	} else if (value.IsMinusInfinity()) {
		sb << "-inf Hz";
	} else if (value.millihertz<int64_t>() % 1000 != 0) {
		sb.AppendFormat("%.3f Hz", value.hertz<double>());
	} else {
		sb << value.hertz<int64_t>() << " Hz";
	}
	return sb.str();
}
}  // namespace mediaserver
