/*********************************************************************************
 *  TotalCross Software Development Kit                                          *
 *  Copyright (C) 2000-2012 SuperWaba Ltda.                                      *
 *  All Rights Reserved                                                          *
 *                                                                               *
 *  This library and virtual machine is distributed in the hope that it will     *
 *  be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of    *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                         *
 *                                                                               *
 *********************************************************************************/



#include "tcvm.h"

static void createClassObject(Context currentContext, CharP className, Object* ret)
{
   Object ptrObj=null;
   TCClass c = loadClass(currentContext, className, true);
   if (c != null && c->classObj != null) // if the object was already created, reuse it.
      *ret = c->classObj; // no need to unlock it
   else
   {
      if (c != null &&
         (*ret = createObject(currentContext, "java.lang.Class")) != null &&
         (ptrObj = createByteArray(currentContext, PTRSIZE)) != null)
      {
         xmoveptr(ARRAYOBJ_START(ptrObj), &c);
         setObjectLock(Class_targetName(*ret) = createStringObjectFromCharP(currentContext,className,-1),UNLOCKED);
      }
      if (ptrObj != null)
      {
         setObjectLock(Class_nativeStruct(*ret) = ptrObj, UNLOCKED);
         if (*ret != null)
            c->classObj = *ret;
      }
      if (*ret != null) // unlock the returning object
         setObjectLock(*ret, UNLOCKED);
   }
}
static void createFieldObject(Context currentContext, Field f, int32 idx, Object* ret)
{
   Object ptrObj=null;
   *ret = null;
   if ((*ret = createObject(currentContext, "java.lang.reflect.Field")) != null)
   {
      // modifiers and index
      int32 mod=0;
      if (f->flags.isFinal    ) mod |= JFLAG_FINAL;
      if (f->flags.isPrivate  ) mod |= JFLAG_PRIVATE;
      if (f->flags.isProtected) mod |= JFLAG_PROTECTED;
      if (f->flags.isPublic   ) mod |= JFLAG_PUBLIC;
      if (f->flags.isStatic   ) mod |= JFLAG_STATIC;
      if (f->flags.isTransient) mod |= JFLAG_TRANSIENT;
      if (f->flags.isVolatile ) mod |= JFLAG_VOLATILE;
      Field_mod(*ret) = mod;
      Field_index(*ret) = idx;
      // field ptr
      ptrObj = createByteArray(currentContext, PTRSIZE);
      if (ptrObj)
         xmoveptr(ARRAYOBJ_START(ptrObj), &f);
      setObjectLock(Field_nativeStruct(*ret) = ptrObj, UNLOCKED);
      // name, type and declaring class
      setObjectLock(Field_name(*ret) = createStringObjectFromCharP(currentContext,f->name,-1),UNLOCKED);
      createClassObject(currentContext, f->targetClassName, &Field_type(*ret));
      createClassObject(currentContext, f->sourceClassName, &Field_declaringClass(*ret));
      setObjectLock(*ret, UNLOCKED);
   }
}

static void createMethodObject(Context currentContext, Method f, TCClass declaringClass, Object* ret, bool isConstructor) // also valid for Constructors
{
   Object ptrObj=null;
   *ret = null;
   if ((*ret = createObject(currentContext, isConstructor ? "java.lang.reflect.Constructor" : "java.lang.reflect.Method")) != null)
   {
      // modifiers
      int32 mod=0,i,n;
      if (f->flags.isFinal    ) mod |= JFLAG_FINAL;
      if (f->flags.isPrivate  ) mod |= JFLAG_PRIVATE;
      if (f->flags.isProtected) mod |= JFLAG_PROTECTED;
      if (f->flags.isPublic   ) mod |= JFLAG_PUBLIC;
      if (f->flags.isStatic   ) mod |= JFLAG_STATIC;
      if (f->flags.isAbstract ) mod |= JFLAG_ABSTRACT;
      Method_mod(*ret) = mod;
      // ptr
      ptrObj = createByteArray(currentContext, PTRSIZE);
      if (ptrObj)
         xmoveptr(ARRAYOBJ_START(ptrObj), &f);
      setObjectLock(Method_nativeStruct(*ret) = ptrObj, UNLOCKED);
      // name and declaring class
      setObjectLock(Method_name(*ret) = createStringObjectFromCharP(currentContext,isConstructor ? declaringClass->name : f->name,-1),UNLOCKED);
      createClassObject(currentContext, declaringClass->name, &Method_declaringClass(*ret));
      // parameters and exceptions
      Method_parameterTypes(*ret) = createArrayObject(currentContext, "java.lang.Class[", n = f->paramCount);
      if (Method_parameterTypes(*ret) && n > 0)
      {
         Object* oa = (Object*)ARRAYOBJ_START(Method_parameterTypes(*ret));
         for (i=0; i < n; i++)
            createClassObject(currentContext, declaringClass->cp->cls[f->cpParams[i]], oa++);
      }
      Method_exceptionTypes(*ret) = createArrayObject(currentContext, "java.lang.Class[", n = ARRAYLENV(f->exceptionHandlers));
      if (Method_exceptionTypes(*ret) && n > 0)
      {
         Object* oa = (Object*)ARRAYOBJ_START(Method_exceptionTypes(*ret));
         for (i=0; i < n; i++)
            createClassObject(currentContext, f->exceptionHandlers[i].className, oa++);
      }

      // return and type
      if (!isConstructor)
      {
         createClassObject(currentContext, declaringClass->cp->cls[f->cpReturn], &Method_returnType(*ret));
         createClassObject(currentContext, f->class_->name, &Method_type(*ret));
      }
      setObjectLock(*ret, UNLOCKED);
   }
}

static void getFieldByName(NMParams p, bool onlyPublic)
{
   Object me = p->obj[0], ret=null;
   Object nameObj = p->obj[1];
   CharP name;
   int32 i,n;
   TCClass c = OBJ_CLASS(me), o;
   FieldArray ff;
   Field found=null;
   if (nameObj == NULL)
      throwException(p->currentContext, NullPointerException,null);
   else
   if ((name = String2CharP(nameObj)) == null)
      throwException(p->currentContext, OutOfMemoryError, null);
   else
   {
      for (o = c; o != null; o = o->superClass)
      {
         for (ff = o->i32InstanceFields, i=0,n = ARRAYLENV(ff); i < n && !found; ff++, i++) if ((!onlyPublic || ff->flags.isPublic) && strEq(ff->name,name)) {found = ff; break;}
         for (ff = o->objInstanceFields, i=0,n = ARRAYLENV(ff); i < n && !found; ff++, i++) if ((!onlyPublic || ff->flags.isPublic) && strEq(ff->name,name)) {found = ff; break;}
         for (ff = o->v64InstanceFields, i=0,n = ARRAYLENV(ff); i < n && !found; ff++, i++) if ((!onlyPublic || ff->flags.isPublic) && strEq(ff->name,name)) {found = ff; break;}
         for (ff = o->i32StaticFields  , i=0,n = ARRAYLENV(ff); i < n && !found; ff++, i++) if ((!onlyPublic || ff->flags.isPublic) && strEq(ff->name,name)) {found = ff; break;}
         for (ff = o->objStaticFields  , i=0,n = ARRAYLENV(ff); i < n && !found; ff++, i++) if ((!onlyPublic || ff->flags.isPublic) && strEq(ff->name,name)) {found = ff; break;}
         for (ff = o->v64StaticFields  , i=0,n = ARRAYLENV(ff); i < n && !found; ff++, i++) if ((!onlyPublic || ff->flags.isPublic) && strEq(ff->name,name)) {found = ff; break;}
      }
      xfree(name);
      if (found)
         createFieldObject(p->currentContext, found, i, &p->retO);
   }
}
static void getMCbyName(NMParams p, CharP methodName, bool isConstructor, bool onlyPublic)
{
   Object me = p->obj[0], ret=null;
   Object classesObj = p->obj[isConstructor ? 1 : 2];
   TCClass c = OBJ_CLASS(me);
   bool found=false;
   int32 i,j;
   int32 nparams = classesObj == null ? 0 : ARRAYOBJ_LEN(classesObj);
   Object* classes = classesObj == null ? null : (Object*)ARRAYOBJ_START(classesObj);
   do
   {
      int32 n = ARRAYLENV(c->methods);
      for (i = 0; i < n; i++)
      {
         Method mm = &c->methods[i];
         CharP mn = mm->name;
         if (onlyPublic && !mm->flags.isPublic)
            continue;
         if (strEq(methodName,mn) && nparams == mm->paramCount)
         {
            bool found = true;
            for (j = 0; j < nparams; j++)  // do NOT invert the loop!
            {
               CharP pt = OBJ_CLASS(classes[j])->name;
               CharP po = c->cp->cls[mm->cpParams[j]];
               if (!strEq(pt,po))
               {
                  found = false;
                  break;
               }
            }
            if (found && (mm->code || mm->flags.isNative)) // not an abstract class?
            {
               createMethodObject(p->currentContext, mm, c, &p->retO, isConstructor);
               break;
            }
         }
      }
      c = c->superClass;
   } while (c && !found);
}
static void getFields(NMParams p, bool onlyPublic)
{
   Object me = p->obj[0], ret=null;
   TCClass c = OBJ_CLASS(me), o;
   int32 count=0,i,n;
   FieldArray ff;
   if (c->flags.isInterface)
   {
      // count how many fields have in this interface and super interfaces
      // an interface has only public static fields.
      for (o = c; o != null; o = o->superClass)
         count += ARRAYLENV(o->i32StaticFields) + ARRAYLENV(o->objStaticFields) + ARRAYLENV(o->v64StaticFields);
      ret = createArrayObject(p->currentContext, "java.lang.reflect.Field[", count);
      if (ret)
      {
         Object* oa = (Object*)ARRAYOBJ_START(ret);
         for (o = c; o != null; o = o->superClass)
         {
            for (ff = o->i32StaticFields, i=0, n = ARRAYLENV(ff); i < n; ff++, i++) createFieldObject(p->currentContext, ff, i, oa++);
            for (ff = o->objStaticFields, i=0, n = ARRAYLENV(ff); i < n; ff++, i++) createFieldObject(p->currentContext, ff, i, oa++);
            for (ff = o->v64StaticFields, i=0, n = ARRAYLENV(ff); i < n; ff++, i++) createFieldObject(p->currentContext, ff, i, oa++);
         }                                                        
      }                                                           
   }                                                              
   else
   {
      // count how many PUBLIC fields have in this class and super classes, depending on the flag
      for (o = c; o != null; o = o->superClass)
      {
         for (ff = o->i32StaticFields  , i=0, n = ARRAYLENV(ff); --n >= 0; ff++) if (!onlyPublic || ff->flags.isPublic) count++;
         for (ff = o->objStaticFields  , i=0, n = ARRAYLENV(ff); --n >= 0; ff++) if (!onlyPublic || ff->flags.isPublic) count++;
         for (ff = o->v64StaticFields  , i=0, n = ARRAYLENV(ff); --n >= 0; ff++) if (!onlyPublic || ff->flags.isPublic) count++;
         for (ff = o->i32InstanceFields, i=0, n = ARRAYLENV(ff); --n >= 0; ff++) if (!onlyPublic || ff->flags.isPublic) count++;
         for (ff = o->objInstanceFields, i=0, n = ARRAYLENV(ff); --n >= 0; ff++) if (!onlyPublic || ff->flags.isPublic) count++;
         for (ff = o->v64InstanceFields, i=0, n = ARRAYLENV(ff); --n >= 0; ff++) if (!onlyPublic || ff->flags.isPublic) count++;
      }
      ret = createArrayObject(p->currentContext, "java.lang.reflect.Field[", count);
      if (ret)
      {
         Object* oa = (Object*)ARRAYOBJ_START(ret);
         for (o = c; o != null; o = o->superClass)
         {
            for (ff = o->i32StaticFields  , i=0, n = ARRAYLENV(ff); i < n; ff++, i++) if (!onlyPublic || ff->flags.isPublic) createFieldObject(p->currentContext, ff, i, oa++);
            for (ff = o->objStaticFields  , i=0, n = ARRAYLENV(ff); i < n; ff++, i++) if (!onlyPublic || ff->flags.isPublic) createFieldObject(p->currentContext, ff, i, oa++);
            for (ff = o->v64StaticFields  , i=0, n = ARRAYLENV(ff); i < n; ff++, i++) if (!onlyPublic || ff->flags.isPublic) createFieldObject(p->currentContext, ff, i, oa++);
            for (ff = o->i32InstanceFields, i=0, n = ARRAYLENV(ff); i < n; ff++, i++) if (!onlyPublic || ff->flags.isPublic) createFieldObject(p->currentContext, ff, i, oa++);
            for (ff = o->objInstanceFields, i=0, n = ARRAYLENV(ff); i < n; ff++, i++) if (!onlyPublic || ff->flags.isPublic) createFieldObject(p->currentContext, ff, i, oa++);
            for (ff = o->v64InstanceFields, i=0, n = ARRAYLENV(ff); i < n; ff++, i++) if (!onlyPublic || ff->flags.isPublic) createFieldObject(p->currentContext, ff, i, oa++);
         }
      }
   }
   setObjectLock(p->retO = ret, UNLOCKED);
}
static void getMCarray(NMParams p, bool isConstructor, bool onlyPublic)
{
   Object me = p->obj[0], ret=null;
   TCClass c = OBJ_CLASS(me), o;
   int32 count=0,n;
   MethodArray ff;
   // count how many PUBLIC fields have in this class and super classes
   for (o = c; o != null; o = o->superClass)
      for (ff = o->methods, n = ARRAYLENV(ff); --n >= 0; ff++) 
         if (!onlyPublic || ff->flags.isPublic) 
         {
            bool isC = strEq(ff->name, CONSTRUCTOR_NAME);
            if (isConstructor == isC)
               count++;
         }
   ret = createArrayObject(p->currentContext, "java.lang.reflect.Method[", count);
   if (ret)
   {
      Object* oa = (Object*)ARRAYOBJ_START(ret);
      for (o = c; o != null; o = o->superClass)
         for (ff = o->methods, n = ARRAYLENV(ff); --n >= 0; ff++) 
            if (!onlyPublic || ff->flags.isPublic) 
            {
               bool isC = strEq(ff->name, CONSTRUCTOR_NAME);
               if (isConstructor == isC)
                  createMethodObject(p->currentContext, ff, o, oa++, isConstructor);
            }
               
   }
   setObjectLock(p->retO = ret, UNLOCKED);
}

//////////////////////////////////////////////////////////////////////////
TC_API void jlC_forName_s(NMParams p) // java/lang/Class native public static Class forName(String className) throws java.lang.ClassNotFoundException;
{
   Object classNameObj = p->obj[0];
   if (classNameObj == NULL)
      throwException(p->currentContext, NullPointerException,null);
   else
   {
      CharP className = String2CharP(classNameObj);
      if (className == null)
         throwException(p->currentContext, OutOfMemoryError, null);
      else
      {
         createClassObject(p->currentContext, className, &p->retO);
         xfree(className);
      }
   }
}
//////////////////////////////////////////////////////////////////////////
TC_API void jlC_newInstance(NMParams p) // java/lang/Class native public Object newInstance() throws java.lang.InstantiationException, java.lang.IllegalAccessException;
{
   TCClass target;
   Object me = p->obj[0];

   xmoveptr(&target, ARRAYOBJ_START(Class_nativeStruct(me)));
   if (target->flags.isInterface || target->flags.isAbstract || target->flags.isArray)
      throwException(p->currentContext, InstantiationException, target->name);
   else
   {
      // check if the default constructor is public
      Method m = getMethod(target, false, CONSTRUCTOR_NAME, 0);
      if (!m || !m->flags.isPublic)
         throwException(p->currentContext, IllegalAccessException, target->name);
      else
      {
         p->retO = createObject(p->currentContext, target->name);
         setObjectLock(p->retO, UNLOCKED);
      }
   }
}
//////////////////////////////////////////////////////////////////////////
TC_API void jlC_isInstance_o(NMParams p) // java/lang/Class native public boolean isInstance(Object obj);
{
   Object me = p->obj[0];
   Object other = p->obj[1];

   if (other == null)
      throwException(p->currentContext, NullPointerException,null);
   else
      p->retI = areClassesCompatible(p->currentContext, OBJ_CLASS(me), OBJ_CLASS(other)->name) == COMPATIBLE;
}
//////////////////////////////////////////////////////////////////////////
TC_API void jlC_isAssignableFrom_c(NMParams p) // java/lang/Class public native boolean isAssignableFrom(Class cls);
{
   Object me = p->obj[0];
   Object cls = p->obj[1];
   if (cls == null)
      throwException(p->currentContext, NullPointerException, "Argument cls");
   else
      p->retI = areClassesCompatible(p->currentContext, OBJ_CLASS(me), OBJ_CLASS(cls)->name) == COMPATIBLE;
}
//////////////////////////////////////////////////////////////////////////
TC_API void jlC_isInterface(NMParams p) // java/lang/Class public native boolean isInterface();
{
   Object me = p->obj[0];
   p->retI = OBJ_CLASS(me)->flags.isInterface;
}
//////////////////////////////////////////////////////////////////////////
TC_API void jlC_isArray(NMParams p) // java/lang/Class public native boolean isArray();
{
   Object me = p->obj[0];
   p->retI = OBJ_CLASS(me)->flags.isArray;
}
//////////////////////////////////////////////////////////////////////////
TC_API void jlC_isPrimitive(NMParams p) // java/lang/Class public native boolean isPrimitive();
{
   Object me = p->obj[0];
   CharP name = OBJ_CLASS(me)->name;
   p->retI = strEq(name,"java.lang.Boolean") || strEq(name,"java.lang.Byte") || strEq(name,"java.lang.Short") ||
      strEq(name,"java.lang.Integer") || strEq(name,"java.lang.Long") || strEq(name,"java.lang.Float") || strEq(name,"java.lang.Double") || strEq(name,"java.lang.Character");
}
//////////////////////////////////////////////////////////////////////////
TC_API void jlC_getSuperclass(NMParams p) // java/lang/Class public native Class getSuperclass();
{
   Object me = p->obj[0];
   TCClass c = OBJ_CLASS(me);
   if (c->superClass != null && !c->flags.isInterface)
      createClassObject(p->currentContext, c->superClass->name, &p->retO);
}
//////////////////////////////////////////////////////////////////////////
TC_API void jlC_getInterfaces(NMParams p) // java/lang/Class public native java.lang.Class[] getInterfaces();
{
   Object me = p->obj[0], ret;
   TCClass c = OBJ_CLASS(me);
   int32 n = ARRAYLENV(c->interfaces),i;
   if (n > 0 && (ret = createArrayObject(p->currentContext, "java.lang.Class[", n)) != null)
   {
      Object* objs = (Object*)ARRAYOBJ_START(ret);
      for (i = 0; i < n; i++)
         createClassObject(p->currentContext, c->interfaces[i]->name, &objs[i]);
      setObjectLock(p->retO = ret, UNLOCKED);
   }
}
//////////////////////////////////////////////////////////////////////////
extern Object *booleanTYPE, *byteTYPE, *shortTYPE, *intTYPE, *longTYPE, *floatTYPE, *doubleTYPE, *charTYPE;
TC_API void jlC_getComponentType(NMParams p) // java/lang/Class public native Class getComponentType();
{
   Object me = p->obj[0];
   TCClass c = OBJ_CLASS(me);
   if (c->flags.isArray)
   {
      CharP name = c->name;
      Type to = type2javaType(c->name);
      switch (to)
      {
         case Type_Byte:    p->retO = *byteTYPE; break;
         case Type_Boolean: p->retO = *booleanTYPE; break;
         case Type_Short:   p->retO = *shortTYPE; break;
         case Type_Char:    p->retO = *charTYPE; break;
         case Type_Int:     p->retO = *intTYPE; break;
         case Type_Long:    p->retO = *longTYPE; break;
         case Type_Float:   p->retO = *floatTYPE; break;
         case Type_Double:  p->retO = *doubleTYPE; break;
         default:
         {
            int32 len = xstrlen(name);
            CharP temp = xmalloc(len);
            if (!temp)
               throwException(p->currentContext, OutOfMemoryError, "To allocate %d bytes", len);
            else
            {
               xmemmove(temp, name, len-1); // cut the last [
               createClassObject(p->currentContext, temp, &p->retO);
               xfree(temp);
            }
         }
      }
   }
}
//////////////////////////////////////////////////////////////////////////
TC_API void jlC_getModifiers(NMParams p) // java/lang/Class public native int getModifiers();
{
   Object me = p->obj[0];
   ClassFlags f = OBJ_CLASS(me)->flags;
   int32 ret = 0;
   if (f.isPublic   ) ret |= JFLAG_PUBLIC;
   if (f.isStatic   ) ret |= JFLAG_STATIC;
   if (f.isFinal    )  ret |= JFLAG_FINAL;
   if (f.isAbstract ) ret |= JFLAG_ABSTRACT;
   if (f.isInterface) ret |= JFLAG_INTERFACE;
   p->retI = ret;
}
//////////////////////////////////////////////////////////////////////////
TC_API void jlC_getFields(NMParams p) // java/lang/Class public native java.lang.reflect.Field[] getFields() throws SecurityException;
{
   getFields(p, true);
}
//////////////////////////////////////////////////////////////////////////
TC_API void jlC_getMethods(NMParams p) // java/lang/Class public native java.lang.reflect.Method[] getMethods() throws SecurityException;
{
   getMCarray(p,false,true);
}
//////////////////////////////////////////////////////////////////////////
TC_API void jlC_getConstructors(NMParams p) // java/lang/Class public native java.lang.reflect.Constructor[] getConstructors() throws SecurityException;
{
   getMCarray(p,true,true);
}
//////////////////////////////////////////////////////////////////////////
TC_API void jlC_getField_s(NMParams p) // java/lang/Class public native java.lang.reflect.Field getField(String name) throws NoSuchFieldException, SecurityException;
{
   getFieldByName(p, true);
}
//////////////////////////////////////////////////////////////////////////
TC_API void jlC_getMethod_sC(NMParams p) // java/lang/Class public native java.lang.reflect.Method getMethod(String name, Class []parameterTypes) throws NoSuchMethodException, SecurityException;
{
   Object nameObj = p->obj[1];
   CharP name;
   if (nameObj == NULL)
      throwException(p->currentContext, NullPointerException,null);
   else
   if ((name = String2CharP(nameObj)) == null)
      throwException(p->currentContext, OutOfMemoryError, null);
   else
   {
      getMCbyName(p, name, false,true);
      xfree(name);
   }
}
//////////////////////////////////////////////////////////////////////////
TC_API void jlC_getConstructor_C(NMParams p) // java/lang/Class public native java.lang.reflect.Constructor getConstructor(Class []parameterTypes) throws NoSuchMethodException, SecurityException;
{
   getMCbyName(p, CONSTRUCTOR_NAME, true,true);
}
//////////////////////////////////////////////////////////////////////////
TC_API void jlC_getDeclaredFields(NMParams p) // java/lang/Class public native java.lang.reflect.Field[] getDeclaredFields() throws SecurityException;
{
   getFields(p, false);
}
//////////////////////////////////////////////////////////////////////////
TC_API void jlC_getDeclaredMethods(NMParams p) // java/lang/Class public native java.lang.reflect.Method[] getDeclaredMethods() throws SecurityException;
{
   getMCarray(p, false, false);
}
//////////////////////////////////////////////////////////////////////////
TC_API void jlC_getDeclaredConstructors(NMParams p) // java/lang/Class public native java.lang.reflect.Constructor[] getDeclaredConstructors() throws SecurityException;
{
   getMCarray(p, true, false);
}
//////////////////////////////////////////////////////////////////////////
TC_API void jlC_getDeclaredField_s(NMParams p) // java/lang/Class public native java.lang.reflect.Field getDeclaredField(String name) throws NoSuchFieldException, SecurityException;
{
   getFieldByName(p, false);
}
//////////////////////////////////////////////////////////////////////////
TC_API void jlC_getDeclaredMethod_sC(NMParams p) // java/lang/Class public native java.lang.reflect.Method getDeclaredMethod(String name, Class []parameterTypes) throws NoSuchMethodException, SecurityException;
{
   Object nameObj = p->obj[1];
   CharP name;
   if (nameObj == NULL)
      throwException(p->currentContext, NullPointerException,null);
   else
   if ((name = String2CharP(nameObj)) == null)
      throwException(p->currentContext, OutOfMemoryError, null);
   else
   {
      getMCbyName(p, name, false,false);
      xfree(name);
   }
}
//////////////////////////////////////////////////////////////////////////
TC_API void jlC_getDeclaredConstructor_C(NMParams p) // java/lang/Class public native java.lang.reflect.Constructor getDeclaredConstructor(Class []parameterTypes) throws NoSuchMethodException, SecurityException;
{
   getMCbyName(p, CONSTRUCTOR_NAME, true,false);
}

#ifdef ENABLE_TEST_SUITE
#include "Class_test.h"
#endif
