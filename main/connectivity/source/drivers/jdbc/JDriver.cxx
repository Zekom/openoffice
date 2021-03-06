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
#include "precompiled_connectivity.hxx"
#include "java/sql/Driver.hxx"
#include "java/lang/Object.hxx"
#include "java/lang/Class.hxx"
#include "java/sql/DriverPropertyInfo.hxx"
#include "java/sql/Connection.hxx"
#include "java/util/Property.hxx"
#include "java/tools.hxx"
#include "connectivity/dbexception.hxx"
#include <jvmfwk/framework.h>
#include "diagnose_ex.h"
#include "resource/jdbc_log.hrc"
#include "resource/common_res.hrc"
#include "resource/sharedresources.hxx"
#include <comphelper/componentcontext.hxx>

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

// -------------------------------------------------------------------------
java_sql_Driver::java_sql_Driver(const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
	:m_aContext( _rxFactory )
    ,m_aLogger( m_aContext.getUNOContext(), "sdbcl", "org.openoffice.sdbc.jdbcBridge" )
{
}
// --------------------------------------------------------------------------------
java_sql_Driver::~java_sql_Driver()
{
}

// static ServiceInfo
//------------------------------------------------------------------------------
rtl::OUString java_sql_Driver::getImplementationName_Static(  ) throw(RuntimeException)
{
	return ::rtl::OUString::createFromAscii("com.sun.star.comp.sdbc.JDBCDriver");
		// this name is referenced in the configuration and in the jdbc.xml
		// Please take care when changing it.
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > java_sql_Driver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
	Sequence< ::rtl::OUString > aSNS( 1 );
	aSNS[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbc.Driver");
	return aSNS;
}
//------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL connectivity::java_sql_Driver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception )
{
	return *(new java_sql_Driver(_rxFactory));
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL java_sql_Driver::getImplementationName(  ) throw(RuntimeException)
{
	return getImplementationName_Static();
}

// --------------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_Driver::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
	Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
	const ::rtl::OUString* pSupported = aSupported.getConstArray();
	const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
	for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
		;

	return pSupported != pEnd;
}

// --------------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL java_sql_Driver::getSupportedServiceNames(  ) throw(RuntimeException)
{
	return getSupportedServiceNames_Static();
}
// -------------------------------------------------------------------------
Reference< XConnection > SAL_CALL java_sql_Driver::connect( const ::rtl::OUString& url, const
														 Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    m_aLogger.log( LogLevel::INFO, STR_LOG_DRIVER_CONNECTING_URL, url );

	Reference< XConnection > xOut;
	if ( acceptsURL(url ) )
	{
		java_sql_Connection* pConnection = new java_sql_Connection( *this );
		xOut = pConnection;
		if ( !pConnection->construct(url,info) )
			xOut.clear(); // an error occured and the java driver didn't throw an exception
        else
            m_aLogger.log( LogLevel::INFO, STR_LOG_DRIVER_SUCCESS );
	}
	return xOut;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL java_sql_Driver::acceptsURL( const ::rtl::OUString& url ) throw(SQLException, RuntimeException)
{
	// don't ask the real driver for the url
	// I feel responsible for all jdbc url's
	sal_Bool bEnabled = sal_False;
    OSL_VERIFY_EQUALS( jfw_getEnabled( &bEnabled ), JFW_E_NONE, "error in jfw_getEnabled" );
	static const ::rtl::OUString s_sJdbcPrefix = ::rtl::OUString::createFromAscii("jdbc:");
	return bEnabled && 0 == url.compareTo(s_sJdbcPrefix, 5);
}
// -------------------------------------------------------------------------
Sequence< DriverPropertyInfo > SAL_CALL java_sql_Driver::getPropertyInfo( const ::rtl::OUString& url,
																		 const Sequence< PropertyValue >& /*info*/ ) throw(SQLException, RuntimeException)
{
	if ( acceptsURL(url) )
	{
		::std::vector< DriverPropertyInfo > aDriverInfo;

		Sequence< ::rtl::OUString > aBooleanValues(2);
        aBooleanValues[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "false" ) );
		aBooleanValues[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "true" ) );

		aDriverInfo.push_back(DriverPropertyInfo(
				::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("JavaDriverClass"))
				,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("The JDBC driver class name."))
				,sal_True
				,::rtl::OUString()
				,Sequence< ::rtl::OUString >())
        );
		aDriverInfo.push_back(DriverPropertyInfo(
				::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("JavaDriverClassPath"))
				,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("The class path where to look for the JDBC driver."))
				,sal_True
				,::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "" ) )
				,Sequence< ::rtl::OUString >())
        );
		aDriverInfo.push_back(DriverPropertyInfo(
				::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SystemProperties"))
				,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Additional properties to set at java.lang.System before loading the driver."))
				,sal_True
				,::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "" ) )
				,Sequence< ::rtl::OUString >())
		);
		aDriverInfo.push_back(DriverPropertyInfo(
				::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParameterNameSubstitution"))
				,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Change named parameters with '?'."))
				,sal_False
				,::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "false" ) )
				,aBooleanValues)
        );
		aDriverInfo.push_back(DriverPropertyInfo(
				::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IgnoreDriverPrivileges"))
				,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Ignore the privileges from the database driver."))
				,sal_False
				,::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "false" ) )
				,aBooleanValues)
        );
		aDriverInfo.push_back(DriverPropertyInfo(
				::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsAutoRetrievingEnabled"))
				,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Retrieve generated values."))
				,sal_False
				,::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "false" ) )
				,aBooleanValues)
        );
        aDriverInfo.push_back(DriverPropertyInfo(
				::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AutoRetrievingStatement"))
				,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Auto-increment statement."))
				,sal_False
				,::rtl::OUString()
				,Sequence< ::rtl::OUString >())
		);
		aDriverInfo.push_back(DriverPropertyInfo(
				::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GenerateASBeforeCorrelationName"))
				,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Generate AS before table correlation names."))
				,sal_False
				,::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "true" ) )
				,aBooleanValues)
		);
        aDriverInfo.push_back(DriverPropertyInfo(
				::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IgnoreCurrency"))
				,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Ignore the currency field from the ResultsetMetaData."))
				,sal_False
				,::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "false" ) )
				,aBooleanValues)
		);
		aDriverInfo.push_back(DriverPropertyInfo(
				::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("EscapeDateTime"))
				,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Escape date time format."))
				,sal_False
				,::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "true" ) )
				,aBooleanValues)
		);
        aDriverInfo.push_back(DriverPropertyInfo(
				::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TypeInfoSettings"))
				,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Defines how the type info of the database metadata should be manipulated."))
				,sal_False
				,::rtl::OUString( )
				,Sequence< ::rtl::OUString > ())
		);
        aDriverInfo.push_back(DriverPropertyInfo(
				::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ImplicitCatalogRestriction"))
				,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("The catalog which should be used in getTables calls, when the caller passed NULL."))
				,sal_False
				,::rtl::OUString( )
				,Sequence< ::rtl::OUString > ())
        );
        aDriverInfo.push_back(DriverPropertyInfo(
				::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ImplicitSchemaRestriction"))
				,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("The schema which should be used in getTables calls, when the caller passed NULL."))
				,sal_False
				,::rtl::OUString( )
				,Sequence< ::rtl::OUString > ())
        );
		return Sequence< DriverPropertyInfo >(&aDriverInfo[0],aDriverInfo.size());
	}
	::connectivity::SharedResources aResources;
    const ::rtl::OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
    ::dbtools::throwGenericSQLException(sMessage ,*this);
	return Sequence< DriverPropertyInfo >();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_Driver::getMajorVersion(  ) throw(RuntimeException)
{
	return 1;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL java_sql_Driver::getMinorVersion(  ) throw(RuntimeException)
{
	return 0;
}
// -------------------------------------------------------------------------


