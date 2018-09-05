#include <malloc.h>
#include "types.h"
#include "result.h"
#include "arm/atomics.h"
#include "kernel/svc.h"
#include "kernel/event.h"
#include "services/nv.h"
#include "nvidia/ioctl.h"
#include "nvidia/buffer.h"
#include "nvidia/address_space.h"
#include "nvidia/channel.h"
#include "nvidia/fence.h"
#include "nvidia/info.h"
#include "nvidia/gpu/cmd_list.h"
#include "nvidia/gpu/gpfifo.h"
#include "nvidia/gpu/zcull_ctx.h"
#include "nvidia/gpu/3d_ctx.h"
#include "nvidia/gpu/error_notifier.h"
#include "nvidia/gpu/gpu.h"

#define DEFAULT_FIFO_ENTRIES 0x800

Result nvGpfifoCreate(NvGpfifo* f, NvChannel* parent)
{
    f->parent = parent;

    NvFence fence;
    Result res = nvioctlChannel_AllocGpfifoEx2(parent->fd, DEFAULT_FIFO_ENTRIES, 1, 0, 0, 0, 0, &fence);
    if (R_SUCCEEDED(res) && (s32)fence.id >= 0)
        nvFenceWait(&fence, -1);
    return res;
}

void nvGpfifoClose(NvGpfifo* f) {
    /**/
}

Result nvGpfifoSubmit(NvGpfifo* f, NvCmdList* cmd_list, NvFence* fence_out)
{
    Result rc;
    nvioctl_gpfifo_entry ent;

    u64 a =
        nvCmdListGetGpuAddr(cmd_list) | (nvCmdListGetListSize(cmd_list) << 42);

    ent.desc32[0] = a;
    ent.desc32[1] = a >> 32;

    fence_out->id = -1;
    fence_out->value = 0;

    rc = nvioctlChannel_SubmitGpfifo(
        f->parent->fd, &ent, 1, /*0x104*/0x104/*flags*/, fence_out);

    cmd_list->num_cmds = 0;

    return rc;
}
