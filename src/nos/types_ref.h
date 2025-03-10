


/*
 File:    NewtonTypes.h

 Contains:  Global types for Newton build system

 Copyright:  � 1992-1996 by Apple Computer, Inc.  All rights reserved.

 Derived from v17 internal.

 */

#ifndef __NEWTONTYPES_H
#define __NEWTONTYPES_H

#ifndef __CONFIGGLOBAL_H
#include "ConfigGlobal.h"
#endif

#if defined(__cplusplus) && defined(applec)
// MPW CFront has a bug
#define CALLVIRTUALDESTRUCTOR(obj, cl) (((cl*)obj)->cl::~cl())
#else
#define CALLVIRTUALDESTRUCTOR(obj, cl) (((cl*)obj)->~cl())
#endif

#if defined(__cplusplus) && !defined(SINGLEOBJECT)

#if defined(__SC__) || !defined(applec)

/* non-MPW CFront compilers (e.g. DOS) need this so our classes will compile */
class SingleObject {};

#define SINGLEOBJECT /* nothing */

#else

#define SINGLEOBJECT : public SingleObject

#endif

#endif


/* If we haven't already done so, pull in stddef.h for size_t */

#ifndef __stddef_h
#include "stddef.h"
#endif


/* Base types */

#ifndef __boolean_defined__
typedef unsigned char  Boolean;
#endif

typedef char      Char;      /* In ARM6 C compiler char is unsigned char */
typedef signed char    SChar;
typedef unsigned char   UChar;

#ifndef __byte_defined__
typedef signed char    Byte;
#endif

typedef signed char    SByte;
typedef unsigned char  UByte;

typedef short      Short;      /* In ANSI C short is signed short */
typedef signed short  SShort;
typedef unsigned short  UShort;

typedef long      Long;      /* In ANSI C long is signed long */
typedef signed long    SLong;
typedef unsigned long  ULong;

typedef signed long    FastInt;

#define FASTINT_MIN    (~0x7fffffff)  /* -2147483648 and 0x80000000 are unsigned */
/* minimum value for an object of type FastInt */
#define  FASTINT_MAX    0x7FFFFFFF    /* maximum value for an object of type FastInt */

#ifndef __unichar_defined__
typedef unsigned short  UniChar;
#endif


typedef UniChar      Character;


/* Pointer types */
#ifndef __ptr_defined__
typedef char*      Ptr;
#endif

#ifndef __handle_defined__
typedef Ptr*      Handle;
#endif

#ifndef __procptr_defined__
typedef long      (*ProcPtr)();
#endif


typedef long NewtonErr;    /* needs to hold error codes less than -32767 */


/* Math types */

typedef struct Int64
{
  SLong  hi;
  ULong  lo;
} Int64;


#ifndef __fixed_defined__
typedef long Fixed;
#endif

#ifndef __fract_defined__
typedef long Fract;
#endif


/* Graphics types */

#ifndef __point_defined__
struct Point
{
  short   v;
  short   h;
};
typedef struct Point Point;
#endif

#ifndef __rect_defined__
struct Rect
{
  short  top;
  short  left;
  short  bottom;
  short  right;
};
typedef struct Rect Rect;
#endif


struct FPoint
{
  Fixed   x;
  Fixed  y;
};
typedef struct FPoint FPoint;


struct FRect
{
  Fixed  left;
  Fixed  top;
  Fixed  right;
  Fixed  bottom;
};
typedef struct FRect FRect;


/* Constants */

#ifndef nil
#define nil    NULL
#endif


#ifndef __boolean_defined__
#if !defined(__arm) || !defined(__cplusplus) /* false and true are now C++ keywords */
#if defined(__arm) /* ARM C compiler warns about using the tokens 'false' and 'true' so hide them */
#define false false_
#define true true_
#endif
enum { false, true };
#endif
#endif


/* Newton memory architecture types */
typedef ULong  VAddr;
typedef ULong  PAddr;
typedef ULong  LAddr;

/* Newton OS types */
typedef ULong  TObjectId;
typedef ULong  HammerIORef;

#endif /* __NEWTONTYPES_H */



/*
 File:    objects.h

 Contains:  Original MP object system
 Basic object interface

 Copyright:  © 1992-1996 by Apple Computer, Inc., all rights reserved.

 Derived from v4 internal, 6/18/96.
 */

#ifndef __OBJECTS_H
#define __OBJECTS_H

#ifndef __CONFIGFRAMES_H
#include "ConfigFrames.h"
#endif

// ---------------------------------------------------------------------------
// A function that takes more than one argument cannot safely take any Ref
// arguments, since it would be possible to invalidate a Ref argument in the
// process of evaluating another argument.  However, to avoid the overhead of
// automatic RefVar creation for critical functions such as EQ, versions are
// available that take Ref arguments (usually called something like EQRef).
// These versions should only be used when all the arguments are simple
// variable references or other expressions that cannot invalidate Refs.
// ---------------------------------------------------------------------------


#ifndef __NEWTON_H
#include "Newton.h"
#endif

#ifdef hasOutOfMemoryErrorNumber
#include "OSErrors.h"    // for kError_No_Memory
#endif

const long kRefTagBits = 2;
const long kRefValueBits = 30;
const long kRefValueMask = -1 << kRefTagBits;
const long kRefTagMask = ~kRefValueMask;
const long kRefImmedBits = 2;
const long kRefImmedMask = -1 << kRefImmedBits;

enum {
  kTagInteger,
  kTagPointer,
  kTagImmed,
  kTagMagicPtr,
  kImmedSpecial = 0,
  kImmedChar,
  kImmedBoolean,
  kImmedReserved
};

typedef long Ref;

#define  MAKEINT(i)      (((long) (i)) << kRefTagBits)
#define  MAKEIMMED(t, v)    ((((((long) (v)) << kRefImmedBits) | ((long) (t))) << kRefTagBits) | kTagImmed)
#define  MAKECHAR(c)      MAKEIMMED(kImmedChar, (unsigned) c)
#define  MAKEBOOLEAN(b)    (b ? TRUEREF : FALSEREF)
#define MAKEMAGICPTR(index)  ((Ref) (((long) (index)) << kRefTagBits) | kTagMagicPtr)

const Ref  NILREF = MAKEIMMED(kImmedSpecial, 0);
const Ref  TRUEREF = MAKEIMMED(kImmedBoolean, 1);
const Ref  FALSEREF = NILREF;  // Thought it might change to a boolean
const Ref  kSymbolClass = MAKEIMMED(kImmedSpecial, 0x5555);
const Ref  kFuncClass = MAKEIMMED(kImmedSpecial, 3);      // Actually encompasses xxxxxx32

#define ISFUNCCLASS(r)  (((r) & 0xFF) == 0x32)
#define FUNCKIND(r)    (((ULong) r) >> 8)

#define  RTAG(r)      (((Ref) (r)) & ~kRefValueMask)
#define  RVALUE(r)    (((Ref) (r)) >> kRefTagBits)
#define RIMMEDTAG(r)  (RVALUE(r) & ~kRefImmedMask)
#define RIMMEDVALUE(r)  (RVALUE(r) >> kRefImmedBits)
#define ISINT(r)    (RTAG(r) == kTagInteger)
#undef ISPTR  // Get rid of the definition from winnt.h
#define ISPTR(r)    ((Boolean)(((Ref) (r)) & 1))
#define  ISIMMED(r)    (RTAG(r) == kTagImmed)
#define  ISCHAR(r)    (ISIMMED(r) && (RIMMEDTAG(r) == kImmedChar))
#define  ISBOOLEAN(r)  (ISIMMED(r) && (RIMMEDTAG(r) == kImmedBoolean))

#define  ISMARK(r)    0  // archaic

extern int _RINTError(Ref r), _RCHARError(Ref r);
inline long  RINT(Ref r)    { return ISINT(r) ? RVALUE(r) : _RINTError(r); }

inline UniChar XRCHAR(Ref r)  { return ISCHAR(r) ? (UniChar)RIMMEDVALUE(r) : _RCHARError(r); }
#define RCHAR(r)  XRCHAR(r)

#define  ISNIL(r)  (((Ref) (r)) == NILREF)
#define  NOTNIL(r)  (((Ref) (r)) != NILREF)
#define  ISFALSE(r)  (((Ref) (r)) == FALSEREF)
#define  ISTRUE(r)  (((Ref) (r)) != FALSEREF)

class TObjectIterator;
struct RefHandle;
class RefVar;
typedef const RefVar& RefArg;

typedef Ref (*MapSlotsFunction)(RefArg tag, RefArg value, ULong anything);


struct RefHandle {
  Ref    ref;
  Ref    stackPos;
};

class RefVar {
public:
  RefHandle* h;

#ifdef __CPPTOOLS_OUT_LINES__
  RefVar();
  ~RefVar();
  RefVar(const Ref r);
  RefVar(const RefVar& o);
  RefVar&  operator=(const RefVar& o);
  RefVar&  operator=(const Ref r);
  operator long() const;
#else
  inline  RefVar();
  inline  ~RefVar();
  inline  RefVar(const Ref r);
  inline  RefVar(const RefVar& o);
  RefVar&  operator=(const RefVar& o)  { h->ref = o.h->ref; return *this; }
  RefVar&  operator=(const Ref r)    { h->ref = r; return *this; }
  operator long() const        { return h->ref; }
#endif

  inline  long  Length()                      const;
  inline  void  SetLength(long length)                const;
  inline  ULong  ObjectFlags()                    const;
  inline  Ref    GetArraySlot(long slot)                const;
  inline  void  SetArraySlot(long slot, RefArg value)        const;
  inline  void  AddArraySlot(RefArg value)              const;
  inline  Ref    GetFrameSlot(RefArg slot)              const;
  inline  void  SetFrameSlot(RefArg slot, RefArg value)        const;
  inline  int    FrameHasSlot(RefArg slot)              const;
  inline  Ref    GetFramePath(RefArg thePath)            const;
  inline  void  SetFramePath(RefArg thePath, RefArg value)      const;
  inline  int    FrameHasPath(RefArg thePath)            const;
  inline  Ref    Clone()                        const;
  inline  void  MapSlots(MapSlotsFunction func, ULong anything)    const;
  inline  TObjectIterator*  NewIterator()              const;
  inline  Ptr    BinaryData()                    const;
  inline  void  Lock()                        const;
  inline  void  Unlock()                      const;
  inline  Ref    Class()                        const;
  inline  void  SetClass(RefArg theClass)              const;
};

int    EQRef(Ref a, Ref b);
inline int EQRefArg(RefArg a, RefArg b) { return EQRef(a, b); }

#ifndef DEFINED_OBJHEADER
#define DEFINED_OBJHEADER
struct ObjHeader {
  char  _[8];
};
#endif

// Public object flag bits
const int kObjSlotted = 1;        // 0 = binary, 1 = slotted
const int kObjFrame = 2;        // if slotted, 0 = array, 1 = frame
// kObjFree      = 0x04,
// kObjMarked    = 0x08,
const int kObjLocked = 16;        // 1 = locked (do not move)
// kObjForward    = 0x20,
const int kObjReadOnly = 64;      // 1 = writing is an error
const int kObjDirty = 128;        // 1 = object has been written to


// the lowest 2 bits indicate the object type,
// distinguished primarily by kObjSlotted
//kBinaryObject  = 0x00,
//kIndirectBinaryObject = 0x02,
//kArrayObject  = kObjSlotted + 0x00,
//kFrameObject  = kObjSlotted + 0x02,
//kObjMask      = 0x03

// To initialize the object system:
//    optionally call SetObjectHeapSize(yourFavoriteSize, long allocateInTempMemory = 1);

void  SetObjectHeapSize(long size, long allocateInTempMemory = 1);
void  InitObjects(void);

Ref    AllocateBinary(RefArg theClass, long length);
Ref    AllocateArray(RefArg theClass, long length);
Ref    AllocateFrame(void);
Ref    AllocateFrameWithMap(RefArg map);
Ref    CollectFrameTags(RefArg frame);

typedef void (*CObjectBinaryProc)(void*);
Ref AllocateCObjectBinary(void* cObj, CObjectBinaryProc destructor
                          , CObjectBinaryProc marker = nil, CObjectBinaryProc updater = nil);

#if defined(__cplusplus) && defined(applec)
#define CALLVIRTUALDESTRCUTOR(obj, cl) (((cl*)obj)->cl::~cl())
#else
#define CALLVIRTUALDESTRCUTOR(obj, cl) (((cl*)obj)->~cl())
#endif

#ifdef hasFramesCObjects
Ref AllocateFramesCObject(long cObjSize, CObjectBinaryProc destructor
                          , CObjectBinaryProc marker = nil, CObjectBinaryProc updater = nil);
#endif


long  Length(Ref obj);  // Length in bytes or slots
void  SetLength(RefArg obj, long length);
ULong  ObjectFlags(Ref obj);
// Get/set for array objects (access by integer)
Ref    GetArraySlotRef(Ref obj, long slot);
inline  Ref    GetArraySlotRefArg(RefArg obj, long slot)  { return GetArraySlotRef(obj, slot); }
void  SetArraySlotRef(Ref obj, long slot, Ref value);
inline  void  SetArraySlotRefArg(RefArg obj, long slot, RefArg value)  { SetArraySlotRef(obj, slot, value); }
void  AddArraySlot(RefArg obj, RefArg value);
// Get/set for frames (access by symbol)
Ref    GetFrameSlotRef(Ref obj, Ref slot);
inline  Ref    GetFrameSlotRefArg(RefArg obj, RefArg slot)  { return GetFrameSlotRef(obj, slot); }
void  SetFrameSlot(RefArg obj, RefArg slot, RefArg value);
int    FrameHasSlotRef(Ref obj, Ref slot);
inline  int    FrameHasSlotRefArg(RefArg obj, RefArg slot)  { return FrameHasSlotRef(obj, slot); }
void  RemoveSlot(RefArg frame, RefArg tag);
// Get/set for frames (access by path)
// Paths are arrays (of class PathExpr) of successive slotnames
// GetFramePath({x: {y: 2}}, [PathExpr: 'x, 'y]) => 2
Ref    GetFramePath(RefArg obj, RefArg thePath);
void  SetFramePath(RefArg obj, RefArg thePath, RefArg value);
int    FrameHasPath(RefArg obj, RefArg thePath);
// Shallow clone of obj
Ref    Clone(RefArg obj);
// Deep clone of obj
Ref    DeepClone(RefArg obj);
// Really deep clone of obj (including maps and ensuring symbols are in RAM)
Ref    TotalClone(RefArg obj);
// Don't clone except as necessary to ensure maps and symbols are in RAM
Ref    EnsureInternal(RefArg obj);
// MapSlots calls a function on each slot of an array or frame object, giving it
// the tag (integer or symbol) and contents of each slot.  "Anything" is passed to
// func.  If func returns anything but NILREF, MapSlots terminates.
void  MapSlots(RefArg obj, MapSlotsFunction func, ULong anything);
// Iterator form of above -- usually should use stack-allocated TObjectIterator instead
TObjectIterator*  NewIterator(RefArg obj);

// These are actually in builtins.c, but they're useful more generally.
void ArrayRemoveCount(RefArg array, FastInt start, FastInt removeCount);
Boolean ArrayRemove(RefArg array, RefArg element);
inline void ArrayAppend(RefArg array, RefArg element)
{ AddArraySlot(array, element); }

extern "C"  Ref FSetContains(RefArg, RefArg array, RefArg target);
extern "C"  Ref FSetAdd(RefArg, RefArg members, RefArg member, RefArg unique);
extern "C"  Ref FSetRemove(RefArg, RefArg members, RefArg member);
extern "C"  Ref FSetOverlaps(RefArg, RefArg array, RefArg targetArray);
extern "C"  Ref FSetUnion(RefArg, RefArg array1, RefArg array2, RefArg unique);
extern "C"  Ref FSetDifference(RefArg, RefArg array1, RefArg array2);

// Make all references to target refer to replacement instead
void  ReplaceObjectRef(Ref target, Ref replacement);
inline  void  ReplaceObjectRefArg(RefArg target, RefArg replacement)  { ReplaceObjectRef(target, replacement); }

// Various heap operations
void  GC();
void  Statistics(ULong* freeSpace, ULong* largestFreeBlock);
Ptr    BlockStatistics(Ptr previousBlock, ULong* nextSize, Boolean* free);
void  HeapBounds(Ptr* start, Ptr* limit);

#ifdef hasObjectConsolidation
void  ConsolidateObjects(Boolean totally);
#endif

void  Uriah();
void  UriahBinaryObjects(int printStrings = false);

// Direct access (must lock/unlock before using pointers)
Ptr    BinaryData(Ref obj);
Ref*  Slots(Ref obj);
void  LockRef(Ref obj);
void  UnlockRef(Ref obj);

// A binary/array object’s class is the value of its class.
// A frame’s class is the value of its class slot.
Ref    ClassOf(RefArg obj);
void  SetClass(RefArg obj, RefArg theClass);

int    IsSubclassRef(Ref sub, Ref super);
inline  int    IsSubclassRefArg(RefArg sub, RefArg super)  { return IsSubclassRef(sub, super); }
int    IsInstance(RefArg obj, RefArg super);

// Symbols (from symbols.c)
Ref    Intern(char* name);      // Create or return a symbol
char*  SymbolName(Ref sym);    // Return a symbol’s name
ULong  SymbolHash(Ref sym);    // Return a symbol's hash value
int    symcmp(char* s1, char* s2);  // Case-insensitive comparison
Ref    CollectAllSymbols(void);  // Return an array of all existing symbols
int    IsSymbol(Ref obj);
int    SymbolCompareLexRef(Ref sym1, Ref sym2);
inline  int    SymbolCompareLexRefArg(RefArg sym1, RefArg sym2)  { return SymbolCompareLexRef(sym1, sym2); }

#define  SYM(name)  Intern(#name)

// Strings
Ref    MakeString(const char* str);
Ref    MakeString(const UniChar* str);    // Unicode version
UniChar*  GetCString(RefArg str);    // Use this one instead

// Returns a new ASCII version (or same object if already ASCII)
Ref    ASCIIString(RefArg str);
Boolean  IsString(RefArg ref);

Boolean IsBinary(RefArg ref);
Boolean IsArray(RefArg ref);
Boolean IsFrame(RefArg ref);
Boolean IsNumber(Ref ref);
Boolean IsPathExpr(RefArg ref);

// Destructive string, array, and binary munging
// s1 - destination string
// s1start - start position [0 - len]
// s1count - size of dest [0 - len-s1start] or -1 meaning len-s1start (i.e., to end)
// s2 - source string or NIL for nothing (in which case start/count are ignored)
// s2start & s2count same as above
void  StrMunger(RefArg s1, long s1start, long s1count,
                RefArg s2, long s2start, long s2count);
void  ArrayMunger(RefArg a1, long a1start, long a1count,
                  RefArg a2, long a2start, long a2count);
void  BinaryMunger(RefArg a1, long a1start, long a1count,
                   RefArg a2, long a2start, long a2count);

int    StrBeginsWith(RefArg str, RefArg prefix);
int    StrEndsWith(RefArg str, RefArg suffix);
void  StrUpcase(RefArg str);
void  StrDowncase(RefArg str);
void  StrCapitalize(RefArg str);
void  StrCapitalizeWords(RefArg str);
void  TrimString(RefArg str);
long  StrPosition(RefArg str, RefArg substr, long startPos);
// test - boolean function object test(item, element), or NIL for EQ test
long  ArrayPosition(RefArg array, RefArg item, long start, RefArg test);
Ref    Substring(RefArg str, long start, long count);
long  StrReplace(RefArg str, RefArg substr, RefArg replacement, long count);

// Sorts an array
// test = '|<|, '|>|, '|str<|, '|str>|, or any function object returning -1,0,1 (as strcmp)
// key = NILREF (use the element directly), or a path, or any function object
void  SortArray(RefArg array, RefArg test, RefArg key);      // in builtins.c

// Real numbers
Ref    MakeReal(double d);
double  CDouble(RefArg d);
long  CoerceToInt(RefArg r);
double  CoerceToDouble(RefArg r);
int    ISREAL(Ref r);

Ptr    ObjectPtr(Ref obj);  // get rid of this

// Misc. GC stuff.

RefHandle*  AllocateRefHandle(Ref targetObj);
void    DisposeRefHandle(RefHandle* Handle);
void    ClearRefHandles();
void    IncrementCurrentStackPos();
void    DecrementCurrentStackPos();

void  ClearGCRoots();
void  AddGCRoot(Ref& root);
void  RemoveGCRoot(Ref& root);

typedef void (*DIYGCFuncPtr)(void*);
void  DIYGCRegister(void* refCon, DIYGCFuncPtr markFunction, DIYGCFuncPtr updateFunction);
void  DIYGCUnregister(void* refCon);
void  DIYGCMark(Ref r);
Ref    DIYGCUpdate(Ref r);

typedef void (*GCProcPtr)(void*);
void  GCRegister(void* refCon, GCProcPtr proc);
void  GCUnregister(void* refCon);

void  ClearGCHooks();

#ifdef hasFramesRainyDayHook
extern  int (*FramesRainyDayHook)(long size);
#endif

#ifdef useRefAccessorArgs
#define ARG Ref
#else
#define ARG RefArg
#endif

inline  int    EQ(ARG a, ARG b)          { return EQRef(a, b); }
inline  Ref    GetArraySlot(ARG a, long b)      { return GetArraySlotRef(a, b); }
inline  void  SetArraySlot(ARG a, long b, ARG c)  { SetArraySlotRef(a, b, c); }
inline  Ref    GetFrameSlot(ARG a, ARG b)      { return GetFrameSlotRef(a, b); }
inline  int    FrameHasSlot(ARG a, ARG b)      { return FrameHasSlotRef(a, b); }
inline  int    IsSubclass(ARG a, ARG b)      { return IsSubclassRef(a, b); }
inline  void  ReplaceObject(ARG a, ARG b)      { ReplaceObjectRef(a, b); }
inline  int    SymbolCompareLex(ARG a, ARG b)    { return SymbolCompareLexRef(a, b); }

#undef ARG

#ifndef __CPPTOOLS_OUT_LINES__

inline  RefVar::RefVar()
{
  h = AllocateRefHandle(NILREF);
}

inline  RefVar::~RefVar()
{
  DisposeRefHandle(h);
}

inline  RefVar::RefVar(const RefVar& o)
{
  h = AllocateRefHandle(o.h->ref);
}

inline  RefVar::RefVar(const Ref r)
{
  h = AllocateRefHandle(r);
}

#endif

inline  long  RefVar::Length()    const
{ return ::Length(h->ref); }

inline  void  RefVar::SetLength(long length)    const
{ ::SetLength(*this, length); }

inline  ULong  RefVar::ObjectFlags()  const
{ return ::ObjectFlags(h->ref); }

inline  Ref    RefVar::GetArraySlot(long slot)    const
{ return ::GetArraySlotRef(h->ref, slot); }

inline  void  RefVar::SetArraySlot(long slot, RefArg value)  const
{ ::SetArraySlotRef(h->ref, slot, value); }

inline  void  RefVar::AddArraySlot(RefArg value)  const
{ ::AddArraySlot(*this, value); }

inline  Ref    RefVar::GetFrameSlot(RefArg slot)  const
{ return ::GetFrameSlotRef(h->ref, slot); }

inline  void  RefVar::SetFrameSlot(RefArg slot, RefArg value)  const
{ ::SetFrameSlot(*this, slot, value); }

inline  int    RefVar::FrameHasSlot(RefArg slot)  const
{ return ::FrameHasSlotRef(h->ref, slot); }

inline  Ref    RefVar::GetFramePath(RefArg thePath)  const
{ return ::GetFramePath(*this, thePath); }

inline  void  RefVar::SetFramePath(RefArg thePath, RefArg value)  const
{ ::SetFramePath(*this, thePath, value); }

inline  int    RefVar::FrameHasPath(RefArg thePath)  const
{ return ::FrameHasPath(*this, thePath); }

inline  Ref    RefVar::Clone()    const
{ return ::Clone(*this); }

inline  void  RefVar::MapSlots(MapSlotsFunction func, ULong anything)  const
{ ::MapSlots(*this, func, anything); }

inline  TObjectIterator*  RefVar::NewIterator()  const
{ return ::NewIterator(*this); }

inline  Ptr    RefVar::BinaryData()    const
{ return ::BinaryData(h->ref); }

inline  void  RefVar::Lock()  const
{ ::LockRef(h->ref); }

inline  void  RefVar::Unlock()    const
{ ::UnlockRef(h->ref); }

inline  Ref    RefVar::Class()  const
{ return ::ClassOf(*this); }

inline  void  RefVar::SetClass(RefArg theClass)  const
{ ::SetClass(*this, theClass); }


class RefStruct : public RefVar {
public:
#ifdef __CPPTOOLS_OUT_LINES__
  RefStruct();
  ~RefStruct();
  RefStruct(const RefVar& o);
  RefStruct(const RefStruct& o);
  RefStruct(const Ref r);
  RefStruct&  operator=(const Ref r);
  RefStruct&  operator=(const RefVar& o);
  RefStruct&  operator=(const RefStruct& o);
  operator long() const;
#else
  inline  RefStruct();
  ~RefStruct()              { }
  inline  RefStruct(const RefVar& o);
  inline  RefStruct(const RefStruct& o);
  inline  RefStruct(const Ref r);
  RefStruct&  operator=(const Ref r)    { h->ref = r; return *this; }
  RefStruct&  operator=(const RefVar& o)  { h->ref = o.h->ref; return *this; }
  RefStruct&  operator=(const RefStruct& o)  { return operator=((const RefVar&) o); }
  operator long() const          { return h->ref; }
#endif
};

#ifndef __CPPTOOLS_OUT_LINES__

inline  RefStruct::RefStruct()
{
  h->stackPos = 0;
}

inline  RefStruct::RefStruct(const RefStruct& o) : RefVar(o)
{
  h->stackPos = 0;
}

inline  RefStruct::RefStruct(const RefVar& o) : RefVar(o)
{
  h->stackPos = 0;
}

inline  RefStruct::RefStruct(const Ref r) : RefVar(r)
{
  h->stackPos = 0;
}

#endif


class TObjectIterator : public SingleObject {
public:
  RefStruct  fTag;
  RefStruct  fValue;
  RefStruct  fObj;

  TObjectIterator(RefArg obj, int includeSiblings = false);
  virtual  ~TObjectIterator();
  void    Reset(void);
  void    Reset(RefArg newObj);
  int      Next(void);
  int      Done(void);
  Ref      Tag(void);
  Ref      Value(void);

private:
  int      fIncludeSiblings;
  long    fIndex;
  long    fLength;
  RefStruct  fMapRef;  // NILREF indicates an Array iterator
};

class TFramesObjectPtr : public SingleObject {
protected:
  RefStruct  fRef;
public:
  TFramesObjectPtr();
  TFramesObjectPtr(Ref r);
  TFramesObjectPtr(const RefStruct& r);
  TFramesObjectPtr(const RefVar& r);
  ~TFramesObjectPtr();
  TFramesObjectPtr(const TFramesObjectPtr& p);
  const TFramesObjectPtr& operator=(const TFramesObjectPtr& p);
  const TFramesObjectPtr& operator=(Ref r);
  operator char*() const;
};

class TBinaryDataPtr : private TFramesObjectPtr {
public:
  TBinaryDataPtr() { }
  TBinaryDataPtr(Ref r) : TFramesObjectPtr(r)  { }
  TBinaryDataPtr(const RefStruct& r) : TFramesObjectPtr(r)  { }
  TBinaryDataPtr(const RefVar& r) : TFramesObjectPtr(r)  { }
  ~TBinaryDataPtr()  { }
  TBinaryDataPtr(const TBinaryDataPtr& p) : TFramesObjectPtr(p)  { }
  const TBinaryDataPtr& operator =(const TBinaryDataPtr& p);
  const TBinaryDataPtr& operator =(Ref r);
  operator char*() const;
};

DeclareException(exOutOfMemory,exRootException);  // evt.ex.outofmem
DeclareBaseException(exRefException);        // type.ref, data is a RefStruct*

void  ThrowRefException(ExceptionName name, RefArg data);
void  ThrowBadTypeWithFrameData(NewtonErr errorCode, RefArg value);

#ifdef hasOutOfMemoryErrorNumber
// On the Newt, exOutOfMemory has a numeric error code
inline void OutOfMemory(char* = 0)
{ throw2(exOutOfMemory, (void*) kError_No_Memory); }
#else
inline void OutOfMemory(char* msg = "out of memory")
{ throw2(exOutOfMemory, msg); }
#endif


#ifdef hasLargeObjects

extern "C" Ref WrapLargeObject(class TStore * store, RefArg oclass, unsigned long id, unsigned long vaddr);
extern "C" Ref FLBAllocCompressed(RefArg rStoreWrapper, RefArg oclass, RefArg length, RefArg rCompanderName, RefArg rCompanderData);
extern "C" Ref FLBAlloc(RefArg rStoreWrapper, RefArg oclass, RefArg length);
extern "C" Ref FLBRollback(RefArg context, RefArg rBinary);
extern "C" Ref FGetBinaryStore(RefArg context, RefArg obj);
extern "C" Ref FGetBinaryCompander(RefArg context, RefArg obj);
extern "C" Ref FGetBinaryCompanderData(RefArg context, RefArg obj);
extern "C" Ref FGetBinaryStoredSize(RefArg context, RefArg obj);

Boolean IsLargeBinary(RefArg ref);
extern "C" Ref GetEntryFromLargeObjectVAddr(unsigned long vaddr);

void MungeLargeBinary(RefArg b, long offset, long count);

void ShutDownLargeObjects();

#endif /*hasLargeObjects*/

// ---------------------------------------------------------------------------
// Somewhat experimental fakeframes (maybe this should be done with a
// preprocessor other than the C++ preprocessor).
//
// To declare:
//
//     DefineFrame3(CFunction, funcPtr, numArgs, docString);
//
// To initialize:
//
//     SetUpBuiltinMap3(CFunction, RSYM(funcptr),
//
// And somewhere you have to define this:
//
//     Ref CFunction::fgPrototype;
//
// ---------------------------------------------------------------------------

extern Ref gClassMap;
Ref    AllocateMapWithTags(RefArg superMap, RefArg tags);


#define SUBMHeader(numTags)                      \
do {                              \
RefVar tags_ = AllocateArray(NILREF, numTags + 1);      \
SetArraySlot(tags_, 0, RSYM(class));

#define SUBMTag(index, tag)                      \
SetArraySlot(tags_, index + 1, tag);

#define SUBMFooter(type)                      \
RefVar map = AllocateMapWithTags(NILREF, tags_);      \
type##::fgPrototype = AllocateFrameWithMap(map);      \
AddGCRoot(type##::fgPrototype);                \
SetArraySlot(type##::fgPrototype, 0, Intern(#type));    \
} while (0);

#define SetUpBuiltinMap1(type, s1)                  \
SUBMHeader(1)                          \
SUBMTag(0, s1)                          \
SUBMFooter(type)

#define SetUpBuiltinMap2(type, s1, s2)                \
SUBMHeader(2)                          \
SUBMTag(0, s1)                          \
SUBMTag(1, s2)                          \
SUBMFooter(type)

#define SetUpBuiltinMap3(type, s1, s2, s3)              \
SUBMHeader(3)                          \
SUBMTag(0, s1)                          \
SUBMTag(1, s2)                          \
SUBMTag(2, s3)                          \
SUBMFooter(type)

#define SetUpBuiltinMap4(type, s1, s2, s3, s4)            \
SUBMHeader(4)                          \
SUBMTag(0, s1)                          \
SUBMTag(1, s2)                          \
SUBMTag(2, s3)                          \
SUBMTag(3, s4)                          \
SUBMFooter(type)

#define SetUpBuiltinMap5(type, s1, s2, s3, s4, s5)          \
SUBMHeader(5)                          \
SUBMTag(0, s1)                          \
SUBMTag(1, s2)                          \
SUBMTag(2, s3)                          \
SUBMTag(3, s4)                          \
SUBMTag(4, s5)                          \
SUBMFooter(type)

#define SetUpBuiltinMap6(type, s1, s2, s3, s4, s5, s6)        \
SUBMHeader(6)                          \
SUBMTag(0, s1)                          \
SUBMTag(1, s2)                          \
SUBMTag(2, s3)                          \
SUBMTag(3, s4)                          \
SUBMTag(4, s5)                          \
SUBMTag(5, s6)                          \
SUBMFooter(type)

#define DefinePtrClass(type)                    \
class type##Ptr : public TFramesObjectPtr {            \
public:                            \
type##Ptr()  { }                        \
type##Ptr(Ref r) : TFramesObjectPtr(r)  { }          \
type##Ptr(const RefStruct& r) : TFramesObjectPtr(r)  { }    \
type##Ptr(const RefVar& r) : TFramesObjectPtr(r)  { }    \
~type##Ptr() { }                      \
type * operator ->() const { return (type *) (char*) *this; }  \
}

#define FrameStructHeader(type)                    \
struct type {                          \
static Ref fgPrototype;                    \
static Ref Allocate() { return Clone(fgPrototype); }    \
ObjHeader  header;                      \
Ref      map;                      \
Ref      classSlot;                    \
Ref

#define DefineFrame1(type, s1)                    \
extern Ref g##type##Map;                    \
struct type;                          \
DefinePtrClass(type);                      \
FrameStructHeader(type) s1;                    \
enum { s_class, s_##s1 };                    \
}

#define DefineFrame2(type, s1, s2)                  \
extern Ref g##type##Map;                    \
struct type;                          \
DefinePtrClass(type);                      \
FrameStructHeader(type) s1, s2;                  \
enum { s_class, s_##s1, s_##s2 };                \
}

#define DefineFrame3(type, s1, s2, s3)                \
extern Ref g##type##Map;                    \
struct type;                          \
DefinePtrClass(type);                      \
FrameStructHeader(type) s1, s2, s3;                \
enum { s_class, s_##s1, s_##s2, s_##s3 };            \
}

#define DefineFrame4(type, s1, s2, s3, s4)              \
extern Ref g##type##Map;                    \
struct type;                          \
DefinePtrClass(type);                      \
FrameStructHeader(type) s1, s2, s3, s4;              \
enum { s_class, s_##s1, s_##s2, s_##s3, s_##s4 };        \
}

#define DefineFrame5(type, s1, s2, s3, s4, s5)            \
extern Ref g##type##Map;                    \
struct type;                          \
DefinePtrClass(type);                      \
FrameStructHeader(type) s1, s2, s3, s4, s5;            \
enum { s_class, s_##s1, s_##s2, s_##s3, s_##s4, s_##s5 };    \
}

#define DefineFrame6(type, s1, s2, s3, s4, s5, s6)          \
extern Ref g##type##Map;                    \
struct type;                          \
DefinePtrClass(type);                      \
FrameStructHeader(type) s1, s2, s3, s4, s5, s6;          \
enum { s_class, s_##s1, s_##s2, s_##s3, s_##s4, s_##s5, s_##s6 };    \
}

#endif // __OBJECTS_H
