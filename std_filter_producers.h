#ifndef SOUND_PROCESSOR_STD_FILTER_PRODUCERS_H
#define SOUND_PROCESSOR_STD_FILTER_PRODUCERS_H

#include "filter_descriptor.h"

namespace FilterProducers {

// каждый продюсер принимает FilterDescriptor и возвращает IFilter*

// factor >= 0
IFilter *amplProducer(const FilterDescriptor &fd);

// peak in [0, 1] (необязательный, по умолчанию 1.0)
IFilter *normalizeProducer(const FilterDescriptor &fd);

// unit = "sec" или "ms"; start >= 0; end >= start
IFilter *silenceProducer(const FilterDescriptor &fd);

// factor > 0
IFilter *timestretchProducer(const FilterDescriptor &fd);

// window_size: нечетное >= 1
IFilter *lowpassProducer(const FilterDescriptor &fd);

// генераторы (имя в дескрипторе без "generator" - парсер его обрабатывает
// отдельно)

// frequencyHz >= 0
IFilter *sinGenProducer(const FilterDescriptor &fd);

// amplitude in [0,1]; carrierHz >= 0; modulationHz >= 0; depth in [0,1]
IFilter *amGenProducer(const FilterDescriptor &fd);

// amplitude in [0,1]; carrierHz >= 0; modulationHz > 0; deviationHz >= 0
IFilter *fmGenProducer(const FilterDescriptor &fd);

} // namespace FilterProducers

#endif // SOUND_PROCESSOR_STD_FILTER_PRODUCERS_H