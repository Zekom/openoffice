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




#ifndef EXTENSIONS_OOOIMPROVEMENT_CONFIGURATIONHELPER_HXX
#define EXTENSIONS_OOOIMPROVEMENT_CONFIGURATIONHELPER_HXX

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <rtl/ustring.hxx>


namespace oooimprovement
{
    #ifdef css
        #error css defined globally
    #endif
    #define css ::com::sun::star
    // Copy from comphelper module, we cant use that directly from an extension
    class MyConfigurationHelper
    {
        public:
            //-----------------------------------------------
            /** specify all possible modes, which can be used to open a configuration access.
             *
             *  @see    openConfig()
             *  @see    readDirectKey()
             *  @see    writeDirectKey()
             */
            enum EConfigurationModes
            {
                /// opens configuration in read/write mode (without LAZY writing!)
                E_STANDARD = 0,
                /// configuration will be opened readonly
                E_READONLY = 1,
                /// all localized nodes will be interpreted as css::uno::XInterface instead of interpreting it as atomic value nodes
                E_ALL_LOCALES = 2,
                /// enable lazy writing
                E_LAZY_WRITE = 4
            };

            //-----------------------------------------------
            /** returns access to the specified configuration package.
             *
             *  This method should be used, if e.g. more then one request to the same
             *  configuration package is needed. The configuration access can be cached
             *  outside and used inbetween.
             *
             *  @param  xSMGR
             *          the uno service manager, which should be used to create the
             *          configuration access.
             *
             *  @param  sPackage
             *          the name of the configuration package.
             *          e.g. <ul>
             *                  <li>org.openoffice.Office.Common</li>
             *                  <li>org.openoffice.Office.Common/Menu</li>
             *               </ul>
             *
             *  @param  eMode
             *          specify the open mode for the returned configuration access.
             *          It's interpreted as a flag field and can be any usefull combination
             *          of values of EConfigurationModes.
             *
             *  @throw  css::uno::Any exceptions the underlying configuration can throw.
             *          E.g. css::uno::Exception if the configuration could not be opened.
             */
            static css::uno::Reference< css::uno::XInterface> openConfig(
                const css::uno::Reference< css::lang::XMultiServiceFactory> xSMGR,
                const ::rtl::OUString& sPackage,
                sal_Int32 eMode);

            //-----------------------------------------------
            /** reads the value of an existing(!) configuration key,
             *  which is searched relative to the specified configuration access.
             *
             *  This method must be used in combination with openConfig().
             *  The cached configuration access must be provided here ... and
             *  all operations are made relativ to this access point.
             *
             *  @param  xCFG
             *          the configuration root, where sRelPath should be interpreted.
             *          as relativ path
             *
             *  @param  sRelPath
             *          path relative to xCFG parameter.
             *
             *  @param  sKey
             *          the configuration node, where we should read the value.
             *
             *  @return [css.uno.css::uno::Any]
             *          the value of sKey.
             *
             *  @throw  css::uno::Any exceptions the underlying configuration can throw.
             *          E.g. css::container::NoSuchElementException if the specified
             *          key does not exists.
             */
            static css::uno::Any readRelativeKey(
                const css::uno::Reference< css::uno::XInterface> xCFG,
                const ::rtl::OUString& sRelPath,
                const ::rtl::OUString& sKey);

        //-----------------------------------------------
        /** writes a new value for an existing(!) configuration key,
         *  which is searched relative to the specified configuration access.
         *
         *  This method must be used in combination with openConfig().
         *  The cached configuration access must be provided here ... and
         *  all operations are made relativ to this access point.
         *
         *  @param  xCFG
         *          the configuration root, where sRelPath should be interpreted.
         *          as relativ path
         *
         *  @param  sRelPath
         *          path relative to xCFG parameter.
         *
         *  @param  sKey
         *          the configuration node, where we should write the new value.
         *
         *  @param  aValue
         *          the new value for sKey.
         *
         *  @throw  css::uno::Any exceptions the underlying configuration can throw.
         *          E.g. css::container::NoSuchElementException if the specified
         *          key does not exists or css::uno::Exception if the provided configuration
         *          access does not allow writing for this key.
         */
        static void writeRelativeKey(
            const css::uno::Reference< css::uno::XInterface> xCFG,
            const ::rtl::OUString& sRelPath,
            const ::rtl::OUString& sKey,
            const css::uno::Any& aValue);

        //-----------------------------------------------
        /** commit all changes made on the specified configuration access.
         *
         *  This method must be used in combination with openConfig().
         *  The cached configuration access must be provided here.
         *
         *  @param  xCFG
         *          the configuration root, where changes should be commited.
         *
         *  @throw  css::uno::Any exceptions the underlying configuration can throw.
         *          E.g. uno::Exception if the provided configuration
         *          access does not allow writing for this set.
         */
        static void flush(const css::uno::Reference< css::uno::XInterface>& xCFG);

        //-----------------------------------------------
        /** does the same then openConfig() & readRelativeKey() together.
         *
         * This method should be used for reading one key at one code place only.
         * Because it opens the specified configuration package, reads the key and
         * closes the configuration again.
         *
         * So its not very usefull to use this method for reading multiple keys at the same time.
         * (Excepting these keys exists inside different configuration packages ...))
         */
        static css::uno::Any readDirectKey(
            const css::uno::Reference< css::lang::XMultiServiceFactory> xSMGR,
            const ::rtl::OUString& sPackage,
            const ::rtl::OUString& sRelPath,
            const ::rtl::OUString& sKey,
            sal_Int32 eMode);

        //-----------------------------------------------
        /** does the same then openConfig() / writeRelativeKey() & flush() together.
         *
         * This method should be used for writing one key at one code place only.
         * Because it opens the specified configuration package, writes the key, flush
         * all changes and closes the configuration again.
         *
         * So its not very usefull to use this method for writing multiple keys at the same time.
         * (Excepting these keys exists inside different configuration packages ...))
         */
        static void writeDirectKey(
            const css::uno::Reference< css::lang::XMultiServiceFactory> xSMGR,
            const ::rtl::OUString& sPackage,
            const ::rtl::OUString& sRelPath,
            const ::rtl::OUString& sKey,
            const css::uno::Any& aValue,
            sal_Int32 eMode);
    };
    #undef css
}
#endif
