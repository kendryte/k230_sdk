/****************************************************************************
*
*    Copyright 2012 - 2022 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include <vg_lite.h>
#include <stdlib.h>
#include <string.h>
#include "vg_lite_context.h"
#ifdef _WIN32
    #include <windows.h>
    #define bool int 
    #define true 1
    #define false 0
#else
    #include <stdbool.h>
    #include <pthread.h>
    #include <unistd.h>
#endif

#if DUMP_CAPTURE
static bool DumpFlag  = 0;
static void * dump_mutex = NULL;
typedef struct _vglitesDUMP_FILE_INFO
{
    FILE *    _debugFile;
    uint32_t   _threadID;
}vglitesDUMP_FILE_INFO;

typedef struct _vglitesBUFFERED_OUTPUT * vglitesBUFFERED_OUTPUT_PTR;
typedef struct _vglitesBUFFERED_OUTPUT
{
    int                         indent;
    vglitesBUFFERED_OUTPUT_PTR  prev;
    vglitesBUFFERED_OUTPUT_PTR  next;
}
vglitesBUFFERED_OUTPUT;

static vglitesBUFFERED_OUTPUT     _outputBuffer[1];
static vglitesBUFFERED_OUTPUT_PTR _outputBufferHead = NULL;
static vglitesBUFFERED_OUTPUT_PTR _outputBufferTail = NULL;

#ifdef __linux__
static pthread_mutex_t _printMutex    = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t _dumpFileMutex = PTHREAD_MUTEX_INITIALIZER;
#endif

/* Alignment with a power of two value. */
#define vglitemALIGN(n, align) \
( \
    ((n) + ((align) - 1)) & ~((align) - 1) \
)

static FILE *_SetDumpFile(
    FILE *File, bool CloseOldFile
    );

vg_lite_error_t
vg_lite_CreateMutex(void **Mutex)
{
#ifdef _WIN32
    void *handle;

    assert(Mutex!=NULL);
    handle = CreateMutex(NULL, 0, NULL);
    
    if (handle == NULL)
    {
        return VG_LITE_OUT_OF_RESOURCES;
    }

    *Mutex = handle;
#else
    pthread_mutex_t* mutex = NULL;
    pthread_mutexattr_t   mta;
    /* Validate the arguments. */
    assert(Mutex != NULL);

    /* Allocate memory for the mutex. */
    mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutexattr_init(&mta);

    pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);

    /* Initialize the mutex. */
    pthread_mutex_init(mutex, &mta);

    /* Destroy mta.*/
    pthread_mutexattr_destroy(&mta);

    /* Return mutex to caller. */
    *Mutex = (void *) mutex;
#endif
    return VG_LITE_SUCCESS;
}

vg_lite_error_t
vg_lite_AcquireMutex(void *Mutex, unsigned int Timeout)
{
    vg_lite_error_t status = VG_LITE_SUCCESS;
#ifdef _WIN32
    unsigned long milliSeconds;

    /* Validate the arguments. */
    assert(Mutex != NULL);

    milliSeconds = (Timeout == ((unsigned int) ~0U))
        ? INFINITE
        : Timeout;

    if (WaitForSingleObject((HANDLE) Mutex,
                            milliSeconds) == WAIT_TIMEOUT)
    {
        /* Timeout. */
        return VG_LITE_TIMEOUT;
    }
#else
    pthread_mutex_t *mutex;
    /* Validate the arguments. */
    assert(Mutex != NULL);

    /* Cast the pointer. */
    mutex = (pthread_mutex_t *) Mutex;

    /* Test for infinite. */
    if (Timeout == ((uint32_t) ~0U))
    {
        /* Lock the mutex. */
        if (pthread_mutex_lock(mutex))
        {
            /* Some error. */
            status = VG_LITE_GENERIC_IO;
        }
        else
        {
            /* Success. */
            status = VG_LITE_SUCCESS;
        }
    }
    else
    {
        /* Try locking the mutex. */
        if (pthread_mutex_trylock(mutex))
        {
            /* Assume timeout. */
            status = VG_LITE_TIMEOUT;

            /* Loop while not timeout. */
            while (Timeout-- > 0)
            {
                /* Try locking the mutex. */
                if (pthread_mutex_trylock(mutex) == 0)
                {
                    /* Success. */
                    status = VG_LITE_SUCCESS;
                    break;
                }

                /* Sleep 1 millisecond. */
                usleep(1000);
            }
        }
        else
        {
            /* Success. */
            status = VG_LITE_SUCCESS;
        }
    }
#endif
    return status;
}

vg_lite_error_t
vg_lite_ReleaseMutex(void *Mutex)
{
#ifdef _WIN32
        /* Validate the arguments. */
    assert(Mutex != NULL);

    /* Release the fast mutex. */
    if (!ReleaseMutex((void *) Mutex))
    {
        return VG_LITE_NOT_SUPPORT;
    }
#else
    pthread_mutex_t *mutex;

    /* Validate the arguments. */
    assert(Mutex != NULL);

    /* Cast the pointer. */
    mutex = (pthread_mutex_t *) Mutex;

    /* Release the mutex. */
    pthread_mutex_unlock(mutex);
#endif
    return VG_LITE_SUCCESS;
}

#define vglitemLOCKDUMP() \
{ \
    if (dump_mutex == NULL) \
    {    \
        assert(vg_lite_CreateMutex(&dump_mutex) == VG_LITE_SUCCESS); \
    } \
    assert(vg_lite_AcquireMutex(dump_mutex, ((unsigned int) ~0U)) == VG_LITE_SUCCESS); \
}

#define vglitemUNLOCKDUMP() \
  assert(vg_lite_ReleaseMutex(dump_mutex) == VG_LITE_SUCCESS);

#define STACK_THREAD_NUMBER 1
static uint32_t            _usedFileSlot = 0;
static uint32_t            _currentPos = 0;
static vglitesDUMP_FILE_INFO    _FileArray[STACK_THREAD_NUMBER];
#define vglitemOPT_VALUE(ptr)    (((ptr) == NULL) ? 0 : *(ptr))

vg_lite_error_t
vg_lite_PrintStrVSafe(
    char * String,
    size_t StringSize,
    unsigned int * Offset,
    const char * Format,
    va_list Arguments
    )
{
    unsigned int offset = vglitemOPT_VALUE(Offset);
    vg_lite_error_t status = VG_LITE_SUCCESS;

    /* Verify the arguments. */
    assert(String != NULL);
    assert(StringSize > 0);
    assert(Format != NULL);

    if (offset < StringSize - 1)
    {
        /* Print into the string. */
        int n = vsnprintf(String + offset,
                             StringSize - offset,
                             Format,
                             Arguments);

        if (n < 0 || n >= (int)(StringSize - offset))
        {
            status = VG_LITE_GENERIC_IO;
        }
        else if (Offset)
        {
            *Offset = offset + n;
        }
    }
    else
    {
        status = VG_LITE_OUT_OF_RESOURCES;
    }

    /* Success. */
    return status;
}


vg_lite_error_t
vg_lite_PrintStrSafe(
    char * String,
    size_t StringSize,
    unsigned int * Offset,
    const char* Format,
    ...
    )
{
    va_list arguments;
    vg_lite_error_t status = VG_LITE_SUCCESS;

    /* Verify the arguments. */
    assert(String != NULL);
    assert(StringSize > 0);
    assert(Format != NULL);


    /* Route through vgliteoOS_PrintStrVSafe. */
    va_start(arguments, Format);
    status = vg_lite_PrintStrVSafe(String, StringSize,
                                   Offset,
                                   Format, arguments);

    va_end(arguments);

    return status;
}

void
vg_lite_SetDebugFile(
    const char* FileName
    )
{
    FILE *debugFile;

    if (FileName != NULL)
    {
        /* Don't change it to 'w' !!!*/
        debugFile = fopen(FileName, "a");
        if (debugFile)
        {
            _SetDumpFile(debugFile, 1);
        }
    }
}

/******************************************************************************\
****************************** OS-dependent Macros *****************************
\******************************************************************************/
#ifdef __linux__
#   define vglitemGETTHREADID() \
    (uint32_t) pthread_self()

#   define vglitemGETPROCESSID() \
    getpid()

#else
#   define vglitemLOCKSECTION() \
    static HANDLE __lockHandle__; \
    \
    if (__lockHandle__ == NULL) \
    { \
        __lockHandle__ = CreateMutex(NULL, 0, NULL); \
    } \
    \
    WaitForSingleObject(__lockHandle__, INFINITE)

#   define vglitemUNLOCKSECTION() \
    ReleaseMutex(__lockHandle__)

#   define vglitemGETPROCESSID() \
    GetCurrentProcessId()

#   define vglitemGETTHREADID() \
    GetCurrentThreadId()
#endif

#   ifdef __linux__
#ifdef __STRICT_ANSI__  /* ANSI C does not have snprintf, vsnprintf functions */
#   define vglitemSPRINTF(Destination, Size, Message, Value) \
        sprintf(Destination, Message, Value)

#   define vglitemVSPRINTF(Destination, Size, Message, Arguments) \
        vsprintf(Destination, Message, Arguments)
#else
#   define vglitemSPRINTF(Destination, Size, Message, Value) \
        snprintf(Destination, Size, Message, Value)

#   define vglitemVSPRINTF(Destination, Size, Message, Arguments) \
        vsnprintf(Destination, Size, Message, Arguments)
#endif

#   define vglitemSTRCAT(Destination, Size, String) \
    strncat(Destination, String, Size)
#else
#   define vglitemSPRINTF(Destination, Size, Message, Value) \
    sprintf_s(Destination, Size, Message, Value)

#   define vglitemVSPRINTF(Destination, Size, Message, Arguments) \
    vsnprintf_s(Destination, Size, Size, Message, Arguments)

#   define vglitemSTRCAT(Destination, Size, String) \
    strcat_s(Destination, Size, String)

#endif

vg_lite_error_t
vgliteoDUMP_SetDumpFlag(
    bool DumpState
    )
{
    DumpFlag = DumpState;

    return VG_LITE_SUCCESS;
}


static FILE *_SetDumpFile(
    FILE *File,
    bool CloseOldFile
    )
{
    FILE *oldFile = NULL;
    uint32_t selfThreadID = vglitemGETTHREADID();
    uint32_t pos;
    uint32_t tmpCurPos;

#ifdef _WIN32
    vglitemLOCKSECTION();
#else 
    pthread_mutex_lock(&_dumpFileMutex);
#endif
    tmpCurPos = _currentPos;

    /* Find if this thread has already been recorded */
    for (pos = 0; pos < _usedFileSlot; pos++)
    {
        if (selfThreadID == _FileArray[pos]._threadID)
        {
            if (_FileArray[pos]._debugFile != NULL &&
                _FileArray[pos]._debugFile != File &&
                CloseOldFile)
            {
                /* Close the earliest existing file handle. */
                fclose(_FileArray[pos]._debugFile);
                _FileArray[pos]._debugFile =  NULL;
            }

            oldFile = _FileArray[pos]._debugFile;
            /* Replace old file by new file */
            _FileArray[pos]._debugFile = File;
            goto exit;
        }
    }

    /* Test if we have exhausted our thread buffers. One thread one buffer. */
    if (tmpCurPos == STACK_THREAD_NUMBER)
    {
        goto error;
    }

    /* Record this new thread */
    _FileArray[tmpCurPos]._debugFile = File;
    _FileArray[tmpCurPos]._threadID = selfThreadID;
    _currentPos = ++tmpCurPos;

    if (_usedFileSlot < STACK_THREAD_NUMBER)
    {
        _usedFileSlot++;
    }

#ifdef __linux__
exit:
    pthread_mutex_unlock(&_dumpFileMutex);
    return oldFile;

error:
    pthread_mutex_unlock(&_dumpFileMutex);
    printf("ERROR: Not enough dump file buffers. Buffer num = %d", STACK_THREAD_NUMBER);
#else
exit:
    vglitemUNLOCKSECTION();
    return oldFile;

error:
    vglitemUNLOCKSECTION();
    printf("ERROR: Not enough dump file buffers. Buffer num = %d", STACK_THREAD_NUMBER);
#endif
    return oldFile;
}

FILE *
_GetDumpFile()
{
    uint32_t selfThreadID;
    uint32_t pos = 0;
    FILE* retFile = NULL;

#ifdef __linux__
    pthread_mutex_lock(&_dumpFileMutex);
#else
    vglitemLOCKSECTION();
#endif

    if (_usedFileSlot == 0)
    {
        goto exit;
    }

    selfThreadID = vglitemGETTHREADID();
    for (; pos < _usedFileSlot; pos++)
    {
        if (selfThreadID == _FileArray[pos]._threadID)
        {
            retFile = _FileArray[pos]._debugFile;
            goto exit;
        }
    }

exit:
#ifdef __linux__
    pthread_mutex_unlock(&_dumpFileMutex); 
#else
    vglitemUNLOCKSECTION();
#endif
    return retFile;
}

#ifdef __linux__
#define vglitemOUTPUT_STRING(File, String) \
    fprintf(((File == NULL) ? stderr : File), "%s", String);
#else
#define vglitemOUTPUT_STRING(File, String) \
    if (File != NULL) \
    { \
        fprintf(File, "%s", String); \
    } \
    else \
    { \
        OutputDebugString(String); \
    }
#endif

static void
OutputString(
    FILE * File,
    const char* String
    )
{
    if (String != NULL)
    {
        vglitemOUTPUT_STRING(File, String);
    }
}

static void
_Print(
    FILE * File,
    const char* Message,
    va_list Arguments
    )
{
    /* Output to file or debugger. */

    int i, j, n, indent;
    static char buffer[4096];
    vglitesBUFFERED_OUTPUT_PTR outputBuffer = NULL;
#ifdef _WIN32
    static uint32_t prevThreadID;
    uint32_t threadID;

    vglitemLOCKSECTION();
    /* Get the current thread ID. */
    threadID = vglitemGETTHREADID();
#else
    pthread_mutex_lock(&_printMutex);
#endif

    /* Initialize output buffer list. */
    if (_outputBufferHead == NULL)
    {
        for (i = 0; i < 1; i += 1)
        {
            if (_outputBufferTail == NULL)
            {
                _outputBufferHead = &_outputBuffer[i];
            }
            else
            {
                _outputBufferTail->next = &_outputBuffer[i];
            }

            _outputBuffer[i].prev = _outputBufferTail;
            _outputBuffer[i].next =  NULL;

            _outputBufferTail = &_outputBuffer[i];
        }
    }

    outputBuffer = _outputBufferHead;

#ifdef _Win32
    /* Update the previous thread value. */
    prevThreadID = threadID;
#endif

    if (strcmp(Message, "$$FLUSH$$") == 0)
    {
        OutputString(File, NULL);
#ifdef _WIN32
    vglitemUNLOCKSECTION();
#else
     pthread_mutex_unlock(&_printMutex);
#endif
        return;
    }

    i = 0;

    if (strncmp(Message, "--", 2) == 0)
    {
        if (outputBuffer->indent == 0)
        {
            OutputString(File,"ERROR: indent=0\n");
        }

        outputBuffer->indent -= 2;
    }

    indent = outputBuffer->indent % 40;

    for (j = 0; j < indent; ++j)
    {
        buffer[i++] = ' ';
    }

    if (indent != outputBuffer->indent)
    {
        i += vglitemSPRINTF(
            buffer + i, sizeof(buffer) - i, " <%d> ", outputBuffer->indent
            );
        buffer[sizeof(buffer) - 1] = '\0';
    }

    /* Print message to buffer. */
    n = vglitemVSPRINTF(buffer + i, sizeof(buffer) - i, Message, Arguments);
    buffer[sizeof(buffer) - 1] = '\0';

    if ((n <= 0) || (buffer[i + n - 1] != '\n'))
    {
        /* Append new-line. */
        vglitemSTRCAT(buffer, sizeof(buffer) - strlen(buffer) - 1, "\n");
        buffer[sizeof(buffer) - 1] = '\0';
    }

    /* Output to debugger. */
    OutputString(File, buffer);

    if (strncmp(Message, "++", 2) == 0)
    {
        outputBuffer->indent += 2;
    }
#ifdef __linux__
    pthread_mutex_unlock(&_printMutex);
#else
    vglitemUNLOCKSECTION();
#endif
}

#define vglitemDEBUGPRINT(FileHandle, Message) \
{ \
    va_list arguments; \
    \
    va_start(arguments, Message); \
    _Print(FileHandle, Message, arguments); \
    va_end(arguments); \
}

void
vgliteoOS_Print(
    const char * Message,
    ...
    )
{
    vglitemDEBUGPRINT(_GetDumpFile(), Message);
}


void _SetDumpFileInfo(
    )
{
#define DUMP_FILE_PREFIX   "hal"

    char dump_file[128];
    unsigned int offset = 0;

    /* Customize filename as needed. */
    vg_lite_PrintStrSafe(dump_file,
        sizeof(dump_file),
        &offset,
        "%s%s_dump_pid-%d_tid-%d_%s.log",
        vgliteDUMP_PATH,
        DUMP_FILE_PREFIX,
#ifdef _WIN32
        (void *)(uintptr_t)(GetCurrentProcessId()),
        (void *)(uintptr_t)GetCurrentThreadId(),
#else
        (void *)(uintptr_t)getpid(),
        (void *)pthread_self(),
#endif
        vgliteDUMP_KEY);

    vg_lite_SetDebugFile(dump_file);
    vgliteoDUMP_SetDumpFlag(1);
}

vg_lite_error_t
vglitefDump(
        char * Message,
        ...
        )
{
    vg_lite_error_t status = VG_LITE_SUCCESS;
    unsigned offset = 0;
    va_list args;
    char buffer[180];

    if (!DumpFlag)
    {
        return status;
    }

    va_start(args, Message);
    status = vg_lite_PrintStrVSafe(buffer, sizeof(buffer),
        &offset,
        Message, args);
    assert(status == VG_LITE_SUCCESS);
    va_end(args);

    vgliteoOS_Print("%s", buffer);;

    return status;
}


vg_lite_error_t
vglitefDumpBuffer(
    char * Tag,
    size_t Physical,
    void * Logical,
    size_t Offset,
    size_t Bytes
    )
{
    unsigned int * ptr = (unsigned int *) Logical + (Offset >> 2);
    size_t bytes   = vglitemALIGN(Bytes + (Offset & 3), 4);

    if (!DumpFlag)
    {
        return VG_LITE_SUCCESS;
    }

    vglitemLOCKDUMP();

    vglitemDUMP("@[%s 0x%08X 0x%08X", Tag, Physical + (Offset & ~3), bytes);

    while (bytes >= 16)
    {
        vglitemDUMP("  0x%08X 0x%08X 0x%08X 0x%08X",
                ptr[0], ptr[1], ptr[2], ptr[3]);

        ptr   += 4;
        bytes -= 16;
    }

    switch (bytes)
    {
    case 12:
        vglitemDUMP("  0x%08X 0x%08X 0x%08X", ptr[0], ptr[1], ptr[2]);
        break;

    case 8:
        vglitemDUMP("  0x%08X 0x%08X", ptr[0], ptr[1]);
        break;

    case 4:
        vglitemDUMP("  0x%08X", ptr[0]);
        break;
    }

    vglitemDUMP("] -- %s", Tag);

    vglitemUNLOCKDUMP();

    return VG_LITE_SUCCESS;
}
#endif
