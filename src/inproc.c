#include "nitro.h"
#include "nitro-private.h"

static nitro_socket_t *bound_inproc_socks;

void inproc_write(nitro_pipe_t *p, nitro_frame_t *f) {
    nitro_socket_t *s = (nitro_socket_t *)p->dest_socket;

    nitro_frame_retain(f);
    pthread_mutex_lock(&s->l_recv);
    DL_APPEND(s->q_recv, f);
    pthread_cond_signal(&s->c_recv);
    s->count_recv++;
    pthread_mutex_unlock(&s->l_recv);
}

static nitro_pipe_t *new_inproc_pipe(nitro_socket_t *orig_socket, nitro_socket_t *dest_socket) {
    nitro_pipe_t *p = calloc(1, sizeof(nitro_pipe_t));
    p->the_socket = (void *)orig_socket;
    p->dest_socket = (void *) dest_socket;
    p->do_write = &inproc_write;
    p->destroy = &destroy_pipe;

    return p;
}

nitro_socket_t * nitro_bind_inproc(char *location) {
    nitro_socket_t *s = nitro_socket_new();
    s->trans = NITRO_SOCKET_INPROC;
    nitro_socket_t *result;
    HASH_FIND(hh, bound_inproc_socks, location, strlen(location), result);
    /* XXX YOU SUCK FOR DOUBLE BINDING */
    assert(!result);
    HASH_ADD_KEYPTR(hh, bound_inproc_socks, location, strlen(location), (nitro_socket_t *)s);
    return s;
}

nitro_socket_t * nitro_connect_inproc(char *location) {
    nitro_socket_t *s = nitro_socket_new();
    s->trans = NITRO_SOCKET_INPROC;
    nitro_socket_t *result;
    HASH_FIND(hh, bound_inproc_socks, location, strlen(location), result);
    /* XXX YOU SUCK FOR LOOKING UP SOMETHING WRONG */
    assert(result);
    if (result) {
        nitro_pipe_t *pipe1 = new_inproc_pipe(s, result);
        CDL_PREPEND(s->pipes, pipe1);
        if (!s->next_pipe) {
            s->next_pipe = s->pipes;
        }
        nitro_pipe_t *pipe2 = new_inproc_pipe(result, s);
        CDL_PREPEND(result->pipes, pipe2);
        if (!result->next_pipe) {
            result->next_pipe = result->pipes;
        }
        
    }

    return s;
}
