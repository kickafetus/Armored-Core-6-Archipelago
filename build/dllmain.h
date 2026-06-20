#pragma once

void LogTagged(const char* tag, const char* format, ...);

#ifndef AC6AP_LOG_TAG
#define AC6AP_LOG_TAG "???"
#endif

#define Log(...) LogTagged(AC6AP_LOG_TAG, __VA_ARGS__)

void QueueGrant(int itemId);
void SetGrantsSafe(bool safe);
void SetGarageVisited();