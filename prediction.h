#ifndef ETL_PREDICTION_H
#define ETL_PREDICTION_H

#define ETL_LIKELY(__c) __builtin_expect(!!(__c), 1)
#define ETL_UNLIKELY(__c) __builtin_expect(!!(__c), 0)

#endif  // ETL_PREDICTION_H
