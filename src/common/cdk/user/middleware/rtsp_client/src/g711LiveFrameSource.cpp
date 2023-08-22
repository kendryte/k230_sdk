#include "g711LiveFrameSource.h"

G711LiveFrameSource* G711LiveFrameSource::createNew(UsageEnvironment &env, size_t queue_size) {
    return new G711LiveFrameSource(env, queue_size);
}

G711LiveFrameSource::G711LiveFrameSource(UsageEnvironment &env, size_t queue_size) : LiveFrameSource(env, queue_size) 
{}
