#include <assert.h>
#include <inttypes.h>
#include <jni.h>
#include <stdlib.h>

#include "othello.h"

static void set_ptr(JNIEnv *env, jobject obj, othello_t *ptr)
{
        jclass class;
        jfieldID field;

        class = (*env)->GetObjectClass(env, obj);
        field = (*env)->GetFieldID(env, class, "mNativeBoard", "J");
        assert(sizeof(jlong) >= sizeof(intptr_t));
        (*env)->SetLongField(env, obj, field, (jlong)(intptr_t)ptr);
}

static othello_t *get_ptr(JNIEnv *env, jobject obj)
{
        jclass class;
        jfieldID field;

        class = (*env)->GetObjectClass(env, obj);
        field = (*env)->GetFieldID(env, class, "mNativeBoard", "J");
        assert(sizeof(jlong) >= sizeof(intptr_t));
        return (othello_t*)(intptr_t)(*env)->GetLongField(env, obj, field);
}

JNIEXPORT void JNICALL
Java_net_hanshq_othello_OthelloBoard_nativeInit(JNIEnv *env, jobject obj)
{
        othello_t *o;

        o = malloc(sizeof(*o));
        othello_init(o);
        set_ptr(env, obj, o);
}

JNIEXPORT void JNICALL
Java_net_hanshq_othello_OthelloBoard_nativeDestroy(JNIEnv *env, jobject obj)
{
        free(get_ptr(env, obj));
        set_ptr(env, obj, NULL);
}

JNIEXPORT void JNICALL
Java_net_hanshq_othello_OthelloBoard_reset(JNIEnv *env, jobject obj)
{
        othello_init(get_ptr(env, obj));
}

JNIEXPORT jint JNICALL
Java_net_hanshq_othello_OthelloBoard_getCellState(JNIEnv *env, jobject obj,
                                                  jint row, jint col){
        return othello_cell_state(get_ptr(env, obj), row, col);
}

JNIEXPORT void JNICALL
Java_net_hanshq_othello_OthelloBoard_setCellState(JNIEnv *env, jobject obj,
                                                  jint row, jint col,
                                                  jint state)
{
        othello_set_cell_state(get_ptr(env, obj), row, col, state);
}

JNIEXPORT jint JNICALL
Java_net_hanshq_othello_OthelloBoard_getScore(JNIEnv *env, jobject obj,
                                              jint player)
{
        return othello_score(get_ptr(env, obj), player);
}

JNIEXPORT jboolean JNICALL
Java_net_hanshq_othello_OthelloBoard_hasValidMove(JNIEnv *env, jobject obj,
                                                  jint player)
{
        return othello_has_valid_move(get_ptr(env, obj), player);
}

JNIEXPORT jboolean JNICALL
Java_net_hanshq_othello_OthelloBoard_isValidMove(JNIEnv *env, jobject obj,
                                                 jint player,
                                                 jint row, jint col)
{
        return othello_is_valid_move(get_ptr(env, obj), player, row, col);
}

JNIEXPORT void JNICALL
Java_net_hanshq_othello_OthelloBoard_makeMove(JNIEnv *env, jobject obj,
                                              jint player, jint row, jint col)
{
        othello_make_move(get_ptr(env, obj), player, row, col);
}

JNIEXPORT jintArray JNICALL
Java_net_hanshq_othello_OthelloBoard_computeMove(JNIEnv *env, jobject obj,
                                                 jint player)
{
        jintArray res;
        int arr[2];

        res = (*env)->NewIntArray(env, 2);
        othello_compute_move(get_ptr(env, obj), player, &arr[0], &arr[1]);
        (*env)->SetIntArrayRegion(env, res, 0, 2, arr);

        return res;
}
