// Taken from https://stackoverflow.com/a/202511
// but added extern "C", enum BaseTypes and const Valid##EnumType##Values[]

// expansion macro for enum value definition
#define ENUM_VALUE(name,assign) name assign,

// expansion macro for enum to string conversion
#define ENUM_CASE(name,assign) case name: return #name;

// expansion macro for string to enum conversion
#define ENUM_STRCMP(name,assign) if (!strcmp(str,#name)) return name;

// expansion macro for const array of valid values
#define ENUM_ARRAYVALUE(name, assign) name,

/// declare the access function and define enum values
#define DECLARE_ENUM(EnumType,BaseType,ENUM_DEF) \
  extern "C" { \
  enum EnumType : BaseType { \
    ENUM_DEF(ENUM_VALUE) \
  }; \
  const char *Get##EnumType##String(EnumType value); \
  EnumType Get##EnumType##Value(const char *str); \
  static const BaseType Valid##EnumType##Values[] = { \
    ENUM_DEF(ENUM_ARRAYVALUE) \
  }; \
  }

/// define the access function names
#define DEFINE_ENUM(EnumType,ENUM_DEF) \
  const char *Get##EnumType##String(EnumType value) \
  { \
    switch(value) \
    { \
      ENUM_DEF(ENUM_CASE) \
      default: return ""; /* handle input error */ \
    } \
  } \
  EnumType Get##EnumType##Value(const char *str) \
  { \
    ENUM_DEF(ENUM_STRCMP) \
    return (EnumType)0; /* handle input error */ \
  }
