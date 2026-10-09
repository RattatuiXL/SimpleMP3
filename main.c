#include <pspkernel.h>
#include <pspaudio.h>
#include <pspmp3.h>
#include <psputility.h>
#include <pspiofilemgr.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>

PSP_MODULE_INFO("SimpleMP3", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER | PSP_THREAD_ATTR_VFPU);

#define MUSIC_DIR "ms0:/MUSIC"
#define MAX_FILES 256
#define IN_SZ  (8 * 1024)
#define OUT_SZ (1152 * 4 * 2)

static char mp3_in[IN_SZ] __attribute__((aligned(64)));
static short pcm_out[OUT_SZ / 2] __attribute__((aligned(64)));
static char files[MAX_FILES][256];
static int nfiles = 0;
static volatile int running = 1;

static int exit_cb(int a, int b, void *c) { running = 0; sceKernelExitGame(); return 0; }
static int cb_thread(SceSize args, void *argp) {
    int id = sceKernelCreateCallback("exit", exit_cb, NULL);
    sceKernelRegisterExitCallback(id);
    sceKernelSleepThreadCB();
    return 0;
}
static void setup_callbacks(void) {
    int t = sceKernelCreateThread("cb", cb_thread, 0x11, 0xFA0, 0, NULL);
    if (t >= 0) sceKernelStartThread(t, 0, NULL);
}

static int fill_stream(int fd, int h) {
    SceUChar8 *dst; SceInt32 want, pos;
    if (sceMp3GetInfoToAddStreamData(h, &dst, &want, &pos) < 0) return -1;
    sceIoLseek32(fd, pos, PSP_SEEK_SET);
    int n = sceIoRead(fd, dst, want);
    if (n <= 0) return 0;
    sceMp3NotifyAddStreamData(h, n);
    return n;
}

static void play(const char *path) {
    int fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fd < 0) return;
    int size = sceIoLseek32(fd, 0, PSP_SEEK_END);
    sceIoLseek32(fd, 0, PSP_SEEK_SET);

    SceMp3InitArg arg;
    memset(&arg, 0, sizeof(arg));
    arg.mp3StreamStart = 0;
    arg.mp3StreamEnd   = size;
    arg.mp3Buf         = mp3_in;
    arg.mp3BufSize     = IN_SZ;
    arg.pcmBuf         = pcm_out;
    arg.pcmBufSize     = OUT_SZ;

    int h = sceMp3ReserveMp3Handle(&arg);
    if (h < 0) { sceIoClose(fd); return; }

    fill_stream(fd, h);
    if (sceMp3Init(h) < 0) { sceMp3ReleaseMp3Handle(h); sceIoClose(fd); return; }

    int rate = sceMp3GetSamplingRate(h);
    sceAudioSRCChReserve(1152, rate, 2);

    while (running) {
        if (sceMp3CheckStreamDataNeeded(h) > 0) fill_stream(fd, h);
        short *out;
        int n = sceMp3Decode(h, &out);
        if (n <= 0) break;
        sceAudioSRCOutputBlocking(PSP_AUDIO_VOLUME_MAX, out);
    }

    sceAudioSRCChRelease();
    sceMp3ReleaseMp3Handle(h);
    sceIoClose(fd);
}

static void scan(void) {
    SceUID d = sceIoDopen(MUSIC_DIR);
    if (d < 0) return;
    SceIoDirent e;
    memset(&e, 0, sizeof(e));
    while (sceIoDread(d, &e) > 0 && nfiles < MAX_FILES) {
        int len = strlen(e.d_name);
        if (len > 4 && strcasecmp(e.d_name + len - 4, ".mp3") == 0)
            snprintf(files[nfiles++], 256, MUSIC_DIR "/%s", e.d_name);
        memset(&e, 0, sizeof(e));
    }
    sceIoDclose(d);
}

int main(void) {
    setup_callbacks();
    sceUtilityLoadModule(PSP_MODULE_AV_AVCODEC);
    sceUtilityLoadModule(PSP_MODULE_AV_MP3);
    sceMp3InitResource();

    scan();
    while (running && nfiles > 0)
        for (int i = 0; i < nfiles && running; i++)
            play(files[i]);

    sceMp3TermResource();
    sceKernelExitGame();
    return 0;
}
