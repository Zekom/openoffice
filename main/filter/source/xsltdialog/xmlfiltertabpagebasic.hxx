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


#ifndef _XMLFILTERTABPAGEBASIC_HXX_
#define _XMLFILTERTABPAGEBASIC_HXX_

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/combobox.hxx>
#include <svtools/svmedit.hxx>

class Window;
class ResMgr;
class filter_info_impl;

class XMLFilterTabPageBasic : public TabPage
{
public:
	XMLFilterTabPageBasic( Window* pParent, ResMgr& rResMgr );
	virtual ~XMLFilterTabPageBasic();

	bool FillInfo( filter_info_impl* pInfo );
	void SetInfo(const filter_info_impl* pInfo);

	static rtl::OUString decodeComment( const rtl::OUString& rComment );
	static rtl::OUString encodeComment( const rtl::OUString& rComment );

	FixedText		maFTFilterName;
	Edit			maEDFilterName;

	FixedText		maFTApplication;
	ComboBox		maCBApplication;

	FixedText		maFTInterfaceName;
	Edit			maEDInterfaceName;

	FixedText		maFTExtension;
	Edit			maEDExtension;
	FixedText		maFTDescription;
	MultiLineEdit	maEDDescription;
};

#endif
