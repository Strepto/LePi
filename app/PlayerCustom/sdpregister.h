#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

class SdpRegisterer{
	public:
		SdpRegisterer();
		sdp_session_t* register_service();
};
