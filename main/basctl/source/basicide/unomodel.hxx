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


#ifndef UNOMODEL_HXX
#define UNOMODEL_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <sfx2/sfxbasemodel.hxx>

//-----------------------------------------------------------------------------
class SIDEModel : public SfxBaseModel,
				public com::sun::star::lang::XServiceInfo
{
public:
	SIDEModel( SfxObjectShell *pObjSh = 0 );
	virtual	~SIDEModel();

	//XInterface
	virtual 	::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw();
    virtual void SAL_CALL release(  ) throw();

	//XTypeProvider
	virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
	virtual rtl::OUString SAL_CALL getImplementationName(void)
		throw( ::com::sun::star::uno::RuntimeException );
	virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
			throw( ::com::sun::star::uno::RuntimeException );
	virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void)
			throw( ::com::sun::star::uno::RuntimeException );

	static ::com::sun::star::uno::Sequence< rtl::OUString > getSupportedServiceNames_Static();
	static ::rtl::OUString getImplementationName_Static();
};

com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL SIDEModel_createInstance(
				const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > & rSMgr )
					throw( com::sun::star::uno::Exception );

#endif
