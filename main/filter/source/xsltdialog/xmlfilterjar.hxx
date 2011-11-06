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


#ifndef _XMLFILTERJAR_HXX_
#define _XMLFILTERJAR_HXX_

#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <vector>

class filter_info_impl;

typedef std::vector< filter_info_impl* > XMLFilterVector;

class XMLFilterJarHelper
{
public:
	XMLFilterJarHelper( com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xMSF );

	bool savePackage( const rtl::OUString& rPackageURL, const XMLFilterVector& rFilters );
	void openPackage( const rtl::OUString& rPackageURL, XMLFilterVector& rFilters );

private:
	void addFile( com::sun::star::uno::Reference< com::sun::star::uno::XInterface > xRootFolder, com::sun::star::uno::Reference< com::sun::star::lang::XSingleServiceFactory > xFactory, const ::rtl::OUString& rSourceFile ) throw( com::sun::star::uno::Exception );

	bool copyFile( com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess > xIfc, ::rtl::OUString& rURL, const ::rtl::OUString& rTargetURL );
	bool copyFiles( com::sun::star::uno::Reference< com::sun::star::container::XHierarchicalNameAccess > xIfc, filter_info_impl* pFilter );

	com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > mxMSF;

	::rtl::OUString sVndSunStarPackage;
	::rtl::OUString sXSLTPath;
	::rtl::OUString sDTDPath;
	::rtl::OUString sTemplatePath;
	::rtl::OUString sSpecialConfigManager;
	::rtl::OUString sPump;
	::rtl::OUString sProgPath;
};

#endif
