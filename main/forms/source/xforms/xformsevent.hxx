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

#ifndef __XFORMSEVENT_HXX
#define __XFORMSEVENT_HXX

#include <sal/types.h>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xforms/XFormsEvent.hpp>
#include <com/sun/star/xml/dom/events/XEventTarget.hpp>

namespace com {
namespace sun {
namespace star {
namespace xforms {  

class XFormsEventConcrete : public cppu::WeakImplHelper1< XFormsEvent > {

	public:
 
		typedef com::sun::star::uno::RuntimeException RuntimeException_t;
		typedef com::sun::star::uno::Reference< com::sun::star::xml::dom::events::XEventTarget > XEventTarget_t;
		typedef com::sun::star::xml::dom::events::PhaseType PhaseType_t;
		typedef com::sun::star::util::Time Time_t;

		inline XFormsEventConcrete( void ) : m_canceled(sal_False) {}
		virtual ~XFormsEventConcrete( void ) {}

		virtual rtl::OUString SAL_CALL getType() throw (RuntimeException_t);
		virtual XEventTarget_t SAL_CALL getTarget() throw (RuntimeException_t);
		virtual XEventTarget_t SAL_CALL getCurrentTarget() throw (RuntimeException_t);
		virtual PhaseType_t SAL_CALL getEventPhase() throw (RuntimeException_t);
		virtual sal_Bool SAL_CALL getBubbles() throw (RuntimeException_t);
		virtual sal_Bool SAL_CALL getCancelable() throw (RuntimeException_t);
		virtual Time_t SAL_CALL getTimeStamp() throw (RuntimeException_t);
		virtual void SAL_CALL stopPropagation() throw (RuntimeException_t);
		virtual void SAL_CALL preventDefault() throw (RuntimeException_t);

		virtual void SAL_CALL initXFormsEvent(
							const rtl::OUString& typeArg, 
							sal_Bool canBubbleArg, 
							sal_Bool cancelableArg )
							throw (RuntimeException_t);

		virtual void SAL_CALL initEvent(
			const rtl::OUString& eventTypeArg, 
			sal_Bool canBubbleArg, 
			sal_Bool cancelableArg) 
			throw (RuntimeException_t);

	private:
		
		sal_Bool m_canceled;

	protected:

		rtl::OUString m_eventType;
		XEventTarget_t m_target;
		XEventTarget_t m_currentTarget;
		PhaseType_t m_phase;
		sal_Bool m_bubbles;
		sal_Bool m_cancelable;
		Time_t m_time;
};

} } } }

#endif
