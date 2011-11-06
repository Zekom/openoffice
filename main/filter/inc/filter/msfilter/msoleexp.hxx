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


#ifndef _MSOLEEXP_HXX
#define _MSOLEEXP_HXX

#include <com/sun/star/uno/Reference.h>
#include "filter/msfilter/msfilterdllapi.h"

// for the CreateSdrOLEFromStorage we need the information, how we handle
// convert able OLE-Objects - this ist stored in
#define OLE_STARMATH_2_MATHTYPE 			0x0001
#define OLE_STARWRITER_2_WINWORD			0x0002
#define OLE_STARCALC_2_EXCEL				0x0004
#define OLE_STARIMPRESS_2_POWERPOINT		0x0008

class SotStorage;

#include <svtools/embedhlp.hxx>

class MSFILTER_DLLPUBLIC SvxMSExportOLEObjects
{
	sal_uInt32 nConvertFlags;
public:
	SvxMSExportOLEObjects( sal_uInt32 nCnvrtFlgs ) : nConvertFlags(nCnvrtFlgs) {}

	void SetFlags( sal_uInt32 n ) 		{ nConvertFlags = n; }
	sal_uInt32 GetFlags() const 		{ return nConvertFlags; }

    void ExportOLEObject( svt::EmbeddedObjectRef& rObj, SotStorage& rDestStg );
    void ExportOLEObject( const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject>& rObj, SotStorage& rDestStg );
};



#endif

