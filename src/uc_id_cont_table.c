#include <sophon_types.h>

static const Sophon_U32 uc_id_cont_table[] = {
	0x00390030, 0x005a0041, 0x005f005f, 0x007a0061, 0x00aa00aa, 0x00b500b5, 0x00ba00ba, 0x00d600c0,
	0x00f600d8, 0x02c100f8, 0x02d102c6, 0x02e402e0, 0x02ec02ec, 0x02ee02ee, 0x03740300, 0x03770376,
	0x037d037a, 0x03860386, 0x038a0388, 0x038c038c, 0x03a1038e, 0x03f503a3, 0x048103f7, 0x04870483,
	0x0523048a, 0x05560531, 0x05590559, 0x05870561, 0x05bd0591, 0x05bf05bf, 0x05c205c1, 0x05c505c4,
	0x05c705c7, 0x05ea05d0, 0x05f205f0, 0x061a0610, 0x065e0621, 0x06690660, 0x06d3066e, 0x06dc06d5,
	0x06e806df, 0x06fc06ea, 0x06ff06ff, 0x074a0710, 0x07b1074d, 0x07f507c0, 0x07fa07fa, 0x09020901,
	0x09390904, 0x093d093c, 0x09480941, 0x094d094d, 0x09540950, 0x09630958, 0x096f0966, 0x09720971,
	0x097f097b, 0x09810981, 0x098c0985, 0x0990098f, 0x09a80993, 0x09b009aa, 0x09b209b2, 0x09b909b6,
	0x09bd09bc, 0x09c409c1, 0x09ce09cd, 0x09dd09dc, 0x09e309df, 0x09f109e6, 0x0a020a01, 0x0a0a0a05,
	0x0a100a0f, 0x0a280a13, 0x0a300a2a, 0x0a330a32, 0x0a360a35, 0x0a390a38, 0x0a3c0a3c, 0x0a420a41,
	0x0a480a47, 0x0a4d0a4b, 0x0a510a51, 0x0a5c0a59, 0x0a5e0a5e, 0x0a750a66, 0x0a820a81, 0x0a8d0a85,
	0x0a910a8f, 0x0aa80a93, 0x0ab00aaa, 0x0ab30ab2, 0x0ab90ab5, 0x0abd0abc, 0x0ac50ac1, 0x0ac80ac7,
	0x0acd0acd, 0x0ad00ad0, 0x0ae30ae0, 0x0aef0ae6, 0x0b010b01, 0x0b0c0b05, 0x0b100b0f, 0x0b280b13,
	0x0b300b2a, 0x0b330b32, 0x0b390b35, 0x0b3d0b3c, 0x0b3f0b3f, 0x0b440b41, 0x0b4d0b4d, 0x0b560b56,
	0x0b5d0b5c, 0x0b630b5f, 0x0b6f0b66, 0x0b710b71, 0x0b830b82, 0x0b8a0b85, 0x0b900b8e, 0x0b950b92,
	0x0b9a0b99, 0x0b9c0b9c, 0x0b9f0b9e, 0x0ba40ba3, 0x0baa0ba8, 0x0bb90bae, 0x0bc00bc0, 0x0bcd0bcd,
	0x0bd00bd0, 0x0bef0be6, 0x0c0c0c05, 0x0c100c0e, 0x0c280c12, 0x0c330c2a, 0x0c390c35, 0x0c400c3d,
	0x0c480c46, 0x0c4d0c4a, 0x0c560c55, 0x0c590c58, 0x0c630c60, 0x0c6f0c66, 0x0c8c0c85, 0x0c900c8e,
	0x0ca80c92, 0x0cb30caa, 0x0cb90cb5, 0x0cbd0cbc, 0x0cbf0cbf, 0x0cc60cc6, 0x0ccd0ccc, 0x0cde0cde,
	0x0ce30ce0, 0x0cef0ce6, 0x0d0c0d05, 0x0d100d0e, 0x0d280d12, 0x0d390d2a, 0x0d3d0d3d, 0x0d440d41,
	0x0d4d0d4d, 0x0d630d60, 0x0d6f0d66, 0x0d7f0d7a, 0x0d960d85, 0x0db10d9a, 0x0dbb0db3, 0x0dbd0dbd,
	0x0dc60dc0, 0x0dca0dca, 0x0dd40dd2, 0x0dd60dd6, 0x0e3a0e01, 0x0e4e0e40, 0x0e590e50, 0x0e820e81,
	0x0e840e84, 0x0e880e87, 0x0e8a0e8a, 0x0e8d0e8d, 0x0e970e94, 0x0e9f0e99, 0x0ea30ea1, 0x0ea50ea5,
	0x0ea70ea7, 0x0eab0eaa, 0x0eb90ead, 0x0ebd0ebb, 0x0ec40ec0, 0x0ec60ec6, 0x0ecd0ec8, 0x0ed90ed0,
	0x0edd0edc, 0x0f000f00, 0x0f190f18, 0x0f290f20, 0x0f350f35, 0x0f370f37, 0x0f390f39, 0x0f470f40,
	0x0f6c0f49, 0x0f7e0f71, 0x0f840f80, 0x0f8b0f86, 0x0f970f90, 0x0fbc0f99, 0x0fc60fc6, 0x102a1000,
	0x1030102d, 0x10371032, 0x103a1039, 0x1049103d, 0x10551050, 0x10611058, 0x10661065, 0x1082106e,
	0x10861085, 0x108e108d, 0x10991090, 0x10c510a0, 0x10fa10d0, 0x10fc10fc, 0x11591100, 0x11a2115f,
	0x11f911a8, 0x12481200, 0x124d124a, 0x12561250, 0x12581258, 0x125d125a, 0x12881260, 0x128d128a,
	0x12b01290, 0x12b512b2, 0x12be12b8, 0x12c012c0, 0x12c512c2, 0x12d612c8, 0x131012d8, 0x13151312,
	0x135a1318, 0x135f135f, 0x138f1380, 0x13f413a0, 0x166c1401, 0x1676166f, 0x169a1681, 0x16ea16a0,
	0x16f016ee, 0x170c1700, 0x1714170e, 0x17341720, 0x17531740, 0x176c1760, 0x1770176e, 0x17731772,
	0x17b31780, 0x17bd17b7, 0x17c617c6, 0x17d317c9, 0x17d717d7, 0x17dd17dc, 0x17e917e0, 0x180d180b,
	0x18191810, 0x18771820, 0x18aa1880, 0x191c1900, 0x19221920, 0x19281927, 0x19321932, 0x193b1939,
	0x196d1946, 0x19741970, 0x19a91980, 0x19c719c1, 0x19d919d0, 0x1a181a00, 0x1b031b00, 0x1b341b05,
	0x1b3a1b36, 0x1b3c1b3c, 0x1b421b42, 0x1b4b1b45, 0x1b591b50, 0x1b731b6b, 0x1b811b80, 0x1ba01b83,
	0x1ba51ba2, 0x1ba91ba8, 0x1bb91bae, 0x1c231c00, 0x1c331c2c, 0x1c371c36, 0x1c491c40, 0x1c7d1c4d,
	0x1de61d00, 0x1f151dfe, 0x1f1d1f18, 0x1f451f20, 0x1f4d1f48, 0x1f571f50, 0x1f591f59, 0x1f5b1f5b,
	0x1f5d1f5d, 0x1f7d1f5f, 0x1fb41f80, 0x1fbc1fb6, 0x1fbe1fbe, 0x1fc41fc2, 0x1fcc1fc6, 0x1fd31fd0,
	0x1fdb1fd6, 0x1fec1fe0, 0x1ff41ff2, 0x1ffc1ff6, 0x2040203f, 0x20542054, 0x20712071, 0x207f207f,
	0x20942090, 0x20dc20d0, 0x20e120e1, 0x20f020e5, 0x21022102, 0x21072107, 0x2113210a, 0x21152115,
	0x211d2119, 0x21242124, 0x21262126, 0x21282128, 0x212d212a, 0x2139212f, 0x213f213c, 0x21492145,
	0x214e214e, 0x21882160, 0x2c2e2c00, 0x2c5e2c30, 0x2c6f2c60, 0x2c7d2c71, 0x2ce42c80, 0x2d252d00,
	0x2d652d30, 0x2d6f2d6f, 0x2d962d80, 0x2da62da0, 0x2dae2da8, 0x2db62db0, 0x2dbe2db8, 0x2dc62dc0,
	0x2dce2dc8, 0x2dd62dd0, 0x2dde2dd8, 0x2dff2de0, 0x2e2f2e2f, 0x30073005, 0x302f3021, 0x30353031,
	0x303c3038, 0x30963041, 0x309a3099, 0x309f309d, 0x30fa30a1, 0x30ff30fc, 0x312d3105, 0x318e3131,
	0x31b731a0, 0x31ff31f0, 0x34003400, 0x4db54db5, 0x4e004e00, 0x9fc39fc3, 0xa48ca000, 0xa60ca500,
	0xa62ba610, 0xa65fa640, 0xa66fa662, 0xa67da67c, 0xa697a67f, 0xa71fa717, 0xa788a722, 0xa78ca78b,
	0xa822a7fb, 0xa826a825, 0xa873a840, 0xa8b3a882, 0xa8c4a8c4, 0xa8d9a8d0, 0xa92da900, 0xa951a930,
	0xaa2eaa00, 0xaa32aa31, 0xaa36aa35, 0xaa4caa40, 0xaa59aa50, 0xac00ac00, 0xd7a3d7a3, 0xfa2df900,
	0xfa6afa30, 0xfad9fa70, 0xfb06fb00, 0xfb17fb13, 0xfb28fb1d, 0xfb36fb2a, 0xfb3cfb38, 0xfb3efb3e,
	0xfb41fb40, 0xfb44fb43, 0xfbb1fb46, 0xfd3dfbd3, 0xfd8ffd50, 0xfdc7fd92, 0xfdfbfdf0, 0xfe0ffe00,
	0xfe26fe20, 0xfe34fe33, 0xfe4ffe4d, 0xfe74fe70, 0xfefcfe76, 0xff19ff10, 0xff3aff21, 0xff3fff3f,
	0xff5aff41, 0xffbeff66, 0xffc7ffc2, 0xffcfffca, 0xffd7ffd2, 0xffdcffda
};