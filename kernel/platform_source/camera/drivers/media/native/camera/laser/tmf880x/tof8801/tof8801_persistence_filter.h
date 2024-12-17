/*
 *****************************************************************************
 * Copyright by ams OSRAM AG                                                 *
 * All rights are reserved.                                                  *
 *                                                                           *
 * IMPORTANT - PLEASE READ CAREFULLY BEFORE COPYING, INSTALLING OR USING     *
 * THE SOFTWARE.                                                             *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT         *
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS         *
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  *
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,     *
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT          *
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     *
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY     *
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT       *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE     *
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.      *
 *****************************************************************************
 */
#include "tof8801.h"

/* Confidence=0 means no object has been found. */
#define TOF8801_NO_OBJECT_CONFIDENCE (0)

/* Confidence=10 means the proximity algorithm (aka short range) reported a
 * distance. */
#define TOF8801_PROX_OBJECT_CONFIDENCE (10)

/* Check if a confidence is low (below a threshold). */
#define TOF8801_IS_LOW_CONFIDENCE(confidence, threshold)                       \
  (((confidence) != TOF8801_NO_OBJECT_CONFIDENCE) &&                           \
   ((confidence) != TOF8801_PROX_OBJECT_CONFIDENCE) &&                         \
   ((confidence) < (threshold)))

/* The persistence filter config and data. */
struct tof8801_persistence_filter
{
  uint8_t count_low;          /* Counter to count the number of successive low
                                 persistence results. */
  uint8_t count_no_obj;       /* Counter to count the number of successive
                                 results that found no object. */
  uint8_t low_conf_thresh;    /* The threshold under which a result
                                 confidence is considered low. */
  uint8_t persistence_thresh; /* The number of sequential results that are
                                 filtered at max. */
  uint16_t old_dist;          /* The previous distance. */
  uint8_t old_conf;           /* The previous confidence. */
};

/**
 * @brief Initialize the filter with parameters, and reset the state.
 *
 * @param filter The filter state.
 * @param low_conf_thresh A confidence below this level is considered low.
 * @param persistence_thresh The number of consequential samples required.
 */
void tof8801_persistence_filter_initialize(struct tof8801_persistence_filter* filter,
                                           uint8_t low_conf_thresh,
                                           uint8_t persistence_thresh);

/**
 * @brief Update the filter with a new result distance and confidence.
 *
 * @param filter The filter state.
 * @param result The result that shall be filtered.
 */
void tof8801_persistence_filter_update(struct tof8801_persistence_filter* filter,
                                       tof_core2_result_t *result);
