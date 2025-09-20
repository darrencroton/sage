#ifndef PARAMETER_TABLE_H
#define PARAMETER_TABLE_H

#include "constants.h"
#include "types.h"

// Parameter definition structure
typedef struct {
  char name[MAX_STRING_LEN];        // Parameter name/tag
  char description[MAX_STRING_LEN]; // Description for documentation
  int type;      // Parameter type (using existing INT, DOUBLE, STRING)
  void *address; // Where to store the value
  int required;  // Whether parameter is required (1) or optional (0)

  // Simple validation for numeric types (ignored for strings)
  double min_value; // Minimum allowed value (for numeric types)
  double max_value; // Maximum allowed value (for numeric types, 0 = no maximum)
} ParameterDefinition;

// Function declarations
extern int get_parameter_table_size(void);
extern ParameterDefinition *get_parameter_table(void);
extern int is_parameter_valid(ParameterDefinition *param, void *value);
extern const char *get_parameter_type_string(int type);

#endif /* #ifndef PARAMETER_TABLE_H */
