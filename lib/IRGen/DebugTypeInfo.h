//===--- DebugTypeInfo.h - Type Info for Debugging --------------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2015 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
// This file defines the data structure that holds all the debug info
// we want to emit for types.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_IRGEN_DEBUGTYPEINFO_H
#define SWIFT_IRGEN_DEBUGTYPEINFO_H

#include "swift/AST/Types.h"
#include "swift/AST/Decl.h"
#include "IRGen.h"

namespace llvm {
  class Type;
}

namespace swift {
  class SILDebugScope;

  namespace irgen {
    class TypeInfo;

    /// This data structure holds everything needed to emit debug info
    /// for a type.
    class DebugTypeInfo {
      public:
      /// The Decl holds the DeclContext, location, but also allows to
      /// look up struct members. If there is no Decl, generic types
      /// mandate there be at least a DeclContext.
      PointerUnion<ValueDecl*, DeclContext*> DeclOrContext;
      /// The type we need to emit may be different from the type
      /// mentioned in the Decl, for example, stripped of qualifiers.
      TypeBase *Type;
      /// Needed to determine the size of basic types.
      // FIXME: Remove this if possible.
      llvm::Type *StorageType;
      Size size;
      Alignment align;
      /// The scope the variable should be emitted in.
      // FIXME: This does not belong here.
      SILDebugScope *DebugScope;

      DebugTypeInfo()
        : Type(nullptr), StorageType(nullptr), size(0), align(1),
          DebugScope(nullptr) {
      }
      DebugTypeInfo(swift::Type Ty, uint64_t SizeInBytes, uint32_t AlignInBytes,
                    DeclContext *DC);
      DebugTypeInfo(swift::Type Ty, Size size, Alignment align,
                    DeclContext *DC);
      DebugTypeInfo(swift::Type Ty, const TypeInfo &Info, DeclContext *DC,
                    SILDebugScope *DS = nullptr);
      DebugTypeInfo(ValueDecl *Decl, const TypeInfo &Info,
                    SILDebugScope *DS = nullptr);
      DebugTypeInfo(ValueDecl *Decl, Size size, Alignment align,
                    SILDebugScope *DS = nullptr);
      DebugTypeInfo(ValueDecl *Decl, swift::Type Ty, const TypeInfo &Info,
                    SILDebugScope *DS = nullptr);
      TypeBase* getHash() const { return getType(); }
      TypeBase* getType() const { return Type; }

      ValueDecl* getDecl() const {
        return DeclOrContext.dyn_cast<ValueDecl*>();
      }

      SILDebugScope *getDebugScope() const { return DebugScope; }
      DeclContext *getDeclContext() const {
        if (ValueDecl *D = getDecl()) return D->getDeclContext();
        else return DeclOrContext.get<DeclContext*>();
      }
      bool isNull() const { return Type == nullptr; }
      bool operator==(DebugTypeInfo T) const;
      bool operator!=(DebugTypeInfo T) const;

      void dump() const;
    };
  }
}

namespace llvm {

  // Dense map specialization.
  template<> struct DenseMapInfo<swift::irgen::DebugTypeInfo> {
    static swift::irgen::DebugTypeInfo getEmptyKey() {
      return swift::irgen::DebugTypeInfo();
    }
    static swift::irgen::DebugTypeInfo getTombstoneKey() {
      return swift::irgen::DebugTypeInfo(llvm::DenseMapInfo<swift::TypeBase*>
                                         ::getTombstoneKey(), 0, 0, 0);
    }
    static unsigned getHashValue(swift::irgen::DebugTypeInfo Val) {
      return DenseMapInfo<swift::CanType>::getHashValue(Val.getType());
    }
    static bool isEqual(swift::irgen::DebugTypeInfo LHS,
                        swift::irgen::DebugTypeInfo RHS) {
      return LHS == RHS;
    }
  };

}

#endif
