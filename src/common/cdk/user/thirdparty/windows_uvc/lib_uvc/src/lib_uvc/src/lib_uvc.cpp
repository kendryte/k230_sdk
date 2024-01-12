#include "lib_uvc.h"
#include "kd_uvc_dev_imp.h"

kd_uvc_dev* kd_create_uvc_dev()
{
	return new kd_uvc_dev_imp();
}

void  kd_destroy_uvc_dev(kd_uvc_dev* p_uvc_dev)
{
	if (p_uvc_dev != nullptr)
	{
		delete p_uvc_dev;
		p_uvc_dev = nullptr;
	}
}
