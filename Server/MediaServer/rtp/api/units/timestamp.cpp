/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#include <rtp/api/units/timestamp.h>

#include <rtp/api/array_view.h>
#include <rtp/base/strings/string_builder.h>

namespace qpidnetwork {
std::string ToString(Timestamp value) {
	char buf[64];
	SimpleStringBuilder sb(buf);
	if (value.IsPlusInfinity()) {
		sb << "+inf ms";
	} else if (value.IsMinusInfinity()) {
		sb << "-inf ms";
	} else {
		if (value.us() == 0 || (value.us() % 1000) != 0)
			sb << value.us() << " us";
		else if (value.ms() % 1000 != 0)
			sb << value.ms() << " ms";
		else
			sb << value.seconds() << " s";
	}
	return sb.str();
}
}  // namespace qpidnetwork
