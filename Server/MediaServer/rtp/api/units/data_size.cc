/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#include <rtp/api/units/data_size.h>

#include <rtp/api/array_view.h>
#include <rtp/base/strings/string_builder.h>

namespace mediaserver {

std::string ToString(DataSize value) {
	char buf[64];
	SimpleStringBuilder sb(buf);
	if (value.IsPlusInfinity()) {
		sb << "+inf bytes";
	} else if (value.IsMinusInfinity()) {
		sb << "-inf bytes";
	} else {
		sb << value.bytes() << " bytes";
	}
	return sb.str();
}
}  // namespace mediaserver
