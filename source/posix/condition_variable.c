/*
 * Copyright 2010-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <aws/common/condition_variable.h>

#include <aws/common/clock.h>
#include <aws/common/mutex.h>

#include <errno.h>

static int process_error_code(int err) {
    switch (err) {
        case ENOMEM:
            return aws_raise_error(AWS_ERROR_OOM);
        case ETIMEDOUT:
            return aws_raise_error(AWS_ERROR_COND_VARIABLE_TIMED_OUT);
        default:
            return aws_raise_error(AWS_ERROR_COND_VARIABLE_ERROR_UNKNOWN);
    }
}

int aws_condition_variable_init(struct aws_condition_variable *condition_variable) {
    if (pthread_cond_init(&condition_variable->condition_handle, NULL)) {
        return aws_raise_error(AWS_ERROR_COND_VARIABLE_INIT_FAILED);
    }

    return AWS_OP_SUCCESS;
}

void aws_condition_variable_clean_up(struct aws_condition_variable *condition_variable) {
    pthread_cond_destroy(&condition_variable->condition_handle);
}

int aws_condition_variable_notify_one(struct aws_condition_variable *condition_variable) {
    int err_code = pthread_cond_signal(&condition_variable->condition_handle);

    if (err_code) {
        return process_error_code(err_code);
    }

    return AWS_OP_SUCCESS;
}

int aws_condition_variable_notify_all(struct aws_condition_variable *condition_variable) {
    int err_code = pthread_cond_broadcast(&condition_variable->condition_handle);

    if (err_code) {
        return process_error_code(err_code);
    }

    return AWS_OP_SUCCESS;
}

int aws_condition_variable_wait(struct aws_condition_variable *condition_variable, struct aws_mutex *mutex) {
    int err_code = pthread_cond_wait(&condition_variable->condition_handle, &mutex->mutex_handle);

    if (err_code) {
        return process_error_code(err_code);
    }

    return AWS_OP_SUCCESS;
}

int aws_condition_variable_wait_for(
    struct aws_condition_variable *condition_variable,
    struct aws_mutex *mutex,
    int64_t time_to_wait) {

    uint64_t current_sys_time = 0;
    if (aws_sys_clock_get_ticks(&current_sys_time)) {
        return AWS_OP_ERR;
    }

    time_to_wait += current_sys_time;

    struct timespec ts;
    uint64_t remainder = 0;
    ts.tv_sec =
        (time_t)aws_timestamp_convert((uint64_t)time_to_wait, AWS_TIMESTAMP_NANOS, AWS_TIMESTAMP_SECS, &remainder);
    ts.tv_nsec = (long)remainder;

    int err_code = pthread_cond_timedwait(&condition_variable->condition_handle, &mutex->mutex_handle, &ts);

    if (err_code) {
        return process_error_code(err_code);
    }

    return AWS_OP_SUCCESS;
}
