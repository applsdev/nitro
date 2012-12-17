#include "nitro.h"
#include "nitro-private.h"

void fatal(char *why) {
    fprintf(stderr, "fatal error: %s\n", why);
}

nitro_counted_buffer * nitro_counted_buffer_new(void *backing) {
    nitro_counted_buffer *buf;
    ZALLOC(buf);
    buf->ptr = backing;
    buf->count = 1;
    pthread_mutex_init(&buf->lock, NULL);

    return buf;
}

void buffer_decref(void *data, void *bufptr) {
    nitro_counted_buffer *buf = (nitro_counted_buffer *)bufptr;

    pthread_mutex_lock(&buf->lock);
    buf->count--;
    if (!buf->count) {
        printf("free!\n");
        free(buf->ptr);
        free(buf);
    }
    else
        pthread_mutex_unlock(&buf->lock);
}

void just_free(void *data, void *unused) {
    free(data);
}