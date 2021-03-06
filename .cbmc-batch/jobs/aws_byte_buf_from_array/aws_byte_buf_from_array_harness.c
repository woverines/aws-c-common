/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <aws/common/byte_buf.h>
#include <proof_helpers/make_common_data_structures.h>

void aws_byte_buf_from_array_harness() {
    /* parameters */
    size_t length;
    uint8_t *array;

    /* assumptions */
    ASSUME_VALID_MEMORY_COUNT(array, length);

    /* operation under verification */
    struct aws_byte_buf buf = aws_byte_buf_from_array(array, length);

    /* assertions */
    assert(aws_byte_buf_is_valid(&buf));
    assert(buf.len == length);
    assert(buf.capacity == length);
    assert(buf.allocator == NULL);
    if (buf.buffer) {
        assert_bytes_match(buf.buffer, array, buf.len);
    }
}
