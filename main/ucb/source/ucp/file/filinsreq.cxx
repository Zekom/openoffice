/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/

 

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"
#include "filinsreq.hxx"
#include "shell.hxx"
#include "filglob.hxx"
#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>



using namespace cppu;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::task;
using namespace com::sun::star::ucb;
using namespace com::sun::star::beans;
using namespace fileaccess;



void SAL_CALL
XInteractionSupplyNameImpl::acquire( void )
	throw()
{
	OWeakObject::acquire();
}



void SAL_CALL
XInteractionSupplyNameImpl::release( void )
	throw()
{
	OWeakObject::release();
}



Any SAL_CALL
XInteractionSupplyNameImpl::queryInterface( const Type& rType )
	throw( RuntimeException )
{
	Any aRet = cppu::queryInterface( rType,
                                     SAL_STATIC_CAST( lang::XTypeProvider*, this ),
                                     SAL_STATIC_CAST( XInteractionSupplyName*,this) );
	return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


//////////////////////////////////////////////////////////////////////////////////////////
//  XTypeProvider
//////////////////////////////////////////////////////////////////////////////////////////

XTYPEPROVIDER_IMPL_2( XInteractionSupplyNameImpl,
					  XTypeProvider,
					  XInteractionSupplyName )



void SAL_CALL
XInteractionAbortImpl::acquire( void )
	throw()
{
	OWeakObject::acquire();
}



void SAL_CALL
XInteractionAbortImpl::release( void )
	throw()
{
	OWeakObject::release();
}



Any SAL_CALL
XInteractionAbortImpl::queryInterface( const Type& rType )
	throw( RuntimeException )
{
	Any aRet = cppu::queryInterface( rType,
                                     SAL_STATIC_CAST( lang::XTypeProvider*, this ),
                                     SAL_STATIC_CAST( XInteractionAbort*,this) );
	return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


//////////////////////////////////////////////////////////////////////////////////////////
//  XTypeProvider
//////////////////////////////////////////////////////////////////////////////////////////

XTYPEPROVIDER_IMPL_2( XInteractionAbortImpl, 
					  XTypeProvider,
					  XInteractionAbort )



XInteractionRequestImpl::XInteractionRequestImpl(
    const rtl::OUString& aClashingName,
    const Reference<XInterface>& xOrigin,
	shell *pShell,sal_Int32 CommandId)
	: p1( new XInteractionSupplyNameImpl ),
	  p2( new XInteractionAbortImpl ),
	  m_nErrorCode(0),
	  m_nMinorError(0),
      m_aSeq( 2 ),
      m_aClashingName(aClashingName),
      m_xOrigin(xOrigin)
{
	if( pShell )
		pShell->retrieveError(CommandId,m_nErrorCode,m_nMinorError);
	m_aSeq[0] = Reference<XInteractionContinuation>(p1);
	m_aSeq[1] = Reference<XInteractionContinuation>(p2);
}

	
void SAL_CALL
XInteractionRequestImpl::acquire( void )
	throw()
{
	OWeakObject::acquire();
}



void SAL_CALL
XInteractionRequestImpl::release( void )
	throw()
{
	OWeakObject::release();
}



Any SAL_CALL
XInteractionRequestImpl::queryInterface( const Type& rType )
	throw( RuntimeException )
{
	Any aRet = 
        cppu::queryInterface( 
            rType,
            SAL_STATIC_CAST( lang::XTypeProvider*, this ),
            SAL_STATIC_CAST( XInteractionRequest*,this) );
	return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


//////////////////////////////////////////////////////////////////////////////////////////
//  XTypeProvider
//////////////////////////////////////////////////////////////////////////////////////////

XTYPEPROVIDER_IMPL_2( XInteractionRequestImpl, 
					  XTypeProvider,
					  XInteractionRequest )
    

Any SAL_CALL
XInteractionRequestImpl::getRequest()
    throw(RuntimeException)
{
	Any aAny;
	if(m_nErrorCode == TASKHANDLING_FOLDER_EXISTS_MKDIR)
	{
		NameClashException excep;
		excep.Name = m_aClashingName;
		excep.Classification = InteractionClassification_ERROR;
		excep.Context = m_xOrigin;
		excep.Message = rtl::OUString(
			RTL_CONSTASCII_USTRINGPARAM(
				"folder exists and overwritte forbidden"));
		aAny <<= excep;
	}
	else if(m_nErrorCode == TASKHANDLING_INVALID_NAME_MKDIR)
	{
		InteractiveAugmentedIOException excep;
		excep.Code = IOErrorCode_INVALID_CHARACTER;
		PropertyValue prop;
		prop.Name = rtl::OUString::createFromAscii("ResourceName");
		prop.Handle = -1;
		prop.Value <<= m_aClashingName;
		Sequence<Any> seq(1);
		seq[0] <<= prop;
		excep.Arguments = seq;
		excep.Classification = InteractionClassification_ERROR;
		excep.Context = m_xOrigin;
		excep.Message = rtl::OUString(
			RTL_CONSTASCII_USTRINGPARAM(
				"the name contained invalid characters"));
		aAny <<= excep;
		
	}
    return aAny;
}
