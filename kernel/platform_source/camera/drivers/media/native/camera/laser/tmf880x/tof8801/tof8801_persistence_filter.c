/*
 *****************************************************************************
 * Copyright by ams OSRAM AG * All rights are reserved. *
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

#include "tof8801_persistence_filter.h"

void tof8801_persistence_filter_initialize(struct tof8801_persistence_filter* filter,
                                           uint8_t low_conf_thresh,
                                           uint8_t persistence_thresh)
{
  filter->count_low = 0;
  filter->count_no_obj = 0;
  filter->low_conf_thresh = low_conf_thresh;
  if (persistence_thresh < 1) {
    filter->persistence_thresh = 1;
  }
  else
  {
    filter->persistence_thresh = persistence_thresh;
  }
  filter->old_conf = TOF8801_NO_OBJECT_CONFIDENCE; // Set no target at startup
  filter->old_dist = 0;
}

void tof8801_persistence_filter_update(struct tof8801_persistence_filter* filter,
                                       tof_core2_result_t *result)
{
  
  if (result->data.resultInfo.reliabilityE == TOF8801_NO_OBJECT_CONFIDENCE) {
    filter->count_low = 0; // Reset all other counters.
    // skip up to persistence_thresh "no-detects" 
    // if a low confidence object was found last.
    if (filter->count_no_obj < (filter->persistence_thresh-1)) {
      filter->count_no_obj++;
      if (TOF8801_IS_LOW_CONFIDENCE(filter->old_conf,
                                    filter->low_conf_thresh)) {
        result->data.resultInfo.reliabilityE = filter->old_conf;
        result->data.distPeak = filter->old_dist;
      }
    }
  } else if (TOF8801_IS_LOW_CONFIDENCE(result->data.resultInfo.reliabilityE, 
                                       filter->low_conf_thresh)) {
    filter->count_no_obj = 0; // Reset all other counters.
    if (filter->count_low < (filter->persistence_thresh-1)) {
      // skip up to persistence_thresh low confidence results
      // if a no object or a prox object was found last.
      // in both cases, report no object
      filter->count_low++;
      if ((filter->old_conf == TOF8801_NO_OBJECT_CONFIDENCE) ||
          (filter->old_conf == TOF8801_PROX_OBJECT_CONFIDENCE)) {
        result->data.resultInfo.reliabilityE = TOF8801_NO_OBJECT_CONFIDENCE;
        result->data.distPeak = 0;
      }
    }
  } else {
    filter->count_low = 0; // Reset all counters.
    filter->count_no_obj = 0;
  }
  filter->old_conf = result->data.resultInfo.reliabilityE;
  filter->old_dist = result->data.distPeak;
}