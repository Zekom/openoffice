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



package ifc.container;

import lib.MultiMethodTest;
import lib.Status;

import com.sun.star.container.XChild;
import com.sun.star.container.XNamed;
import com.sun.star.uno.UnoRuntime;

/*
* Testing <code>com.sun.star.container.XChild</code>
* interface methods :
* <ul>
*  <li><code> getParent()</code></li>
*  <li><code> setParent()</code></li>
* </ul>
* @see com.sun.star.container.XChild
*/
public class _XChild extends MultiMethodTest {

    public XChild oObj = null;
    public Object gotten = null;


    /**
    * Test calls the method and checks return value and that
    * no exceptions were thrown. Parent returned is stored.<p>
    * Has <b> OK </b> status if the method returns not null value
    * and no exceptions were thrown. <p>
    */
    public void _getParent() {
        gotten = oObj.getParent();
        XNamed the_name = (XNamed) UnoRuntime.queryInterface(XNamed.class,gotten);
        if (the_name != null)
            log.println("Parent:"+the_name.getName());
        tRes.tested("getParent()",gotten != null);
    }

    /**
    * Sets existing parent and checks that
    * no exceptions were thrown. <p>
    * Has <b> OK </b> status if no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getParent() </code> : to get the parent. </li>
    * </ul>
    */
    public void _setParent() {
        requiredMethod("getParent()") ;
        
        String parentComment = (String) tEnv.getObjRelation("cannotSwitchParent");
        
        if (parentComment != null) {
            log.println(parentComment);
            tRes.tested("setParent()",Status.skipped(true));
            return;
        }

        try {
            oObj.setParent(gotten);
            tRes.tested("setParent()",true);
        }
        catch (com.sun.star.lang.NoSupportException ex) {
            log.println("Exception occured during setParent()");
            ex.printStackTrace(log);
            tRes.tested("setParent()",false);
        }

    }

}  // finish class _XChild


