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
#include "precompiled_sw.hxx"
#include <anchoreddrawobject.hxx>
#include <svx/svdobj.hxx>
#include <dcontact.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <doc.hxx>
#include <tocntntanchoredobjectposition.hxx>
#include <tolayoutanchoredobjectposition.hxx>
#include <frmtool.hxx>
#include <fmtornt.hxx>
// --> OD 2004-08-12 #i32795#
#include <txtfrm.hxx>
// <--
// --> OD 2004-08-12 #i32795#
// template class <std::vector>
#include <vector>
// <--

// --> OD 2004-08-10 #i28749#
#include <com/sun/star/text/PositionLayoutDir.hpp>
// <--
// --> OD 2005-03-09 #i44559#
#include <ndtxt.hxx>
// <--

using namespace ::com::sun::star;

// ============================================================================
// helper class for correct notification due to the positioning of
// the anchored drawing object
// ============================================================================
class SwPosNotify
{
    private:
        SwAnchoredDrawObject* mpAnchoredDrawObj;
        SwRect maOldObjRect;
        SwPageFrm* mpOldPageFrm;

    public:
        SwPosNotify( SwAnchoredDrawObject* _pAnchoredDrawObj );
        ~SwPosNotify();
        // --> OD 2004-08-12 #i32795#
        Point LastObjPos() const;
        // <--
};

SwPosNotify::SwPosNotify( SwAnchoredDrawObject* _pAnchoredDrawObj ) :
    mpAnchoredDrawObj( _pAnchoredDrawObj )
{
    maOldObjRect = mpAnchoredDrawObj->GetObjRect();
    // --> OD 2004-10-20 #i35640# - determine correct page frame
    mpOldPageFrm = mpAnchoredDrawObj->GetPageFrm();
    // <--
}

SwPosNotify::~SwPosNotify()
{
    if ( maOldObjRect != mpAnchoredDrawObj->GetObjRect() )
    {
        if( maOldObjRect.HasArea() && mpOldPageFrm )
        {
            mpAnchoredDrawObj->NotifyBackground( mpOldPageFrm, maOldObjRect,
                                                 PREP_FLY_LEAVE );
        }
        SwRect aNewObjRect( mpAnchoredDrawObj->GetObjRect() );
        if( aNewObjRect.HasArea() )
        {
            // --> OD 2004-10-20 #i35640# - determine correct page frame
            SwPageFrm* pNewPageFrm = mpAnchoredDrawObj->GetPageFrm();
            // <--
            if( pNewPageFrm )
                mpAnchoredDrawObj->NotifyBackground( pNewPageFrm, aNewObjRect,
                                                     PREP_FLY_ARRIVE );
        }

        ::ClrContourCache( mpAnchoredDrawObj->GetDrawObj() );

        // --> OD 2004-10-20 #i35640# - additional notify anchor text frame
        // Needed for negative positioned drawing objects
        // --> OD 2005-03-01 #i43255# - refine condition to avoid unneeded
        // invalidations: anchored object had to be on the page of its anchor
        // text frame.
        if ( mpAnchoredDrawObj->GetAnchorFrm()->IsTxtFrm() &&
             mpOldPageFrm == mpAnchoredDrawObj->GetAnchorFrm()->FindPageFrm() )
        {
            mpAnchoredDrawObj->AnchorFrm()->Prepare( PREP_FLY_LEAVE );
        }
        // <--

        // indicate a restart of the layout process
        mpAnchoredDrawObj->SetRestartLayoutProcess( true );
    }
    else
    {
        // lock position
        mpAnchoredDrawObj->LockPosition();

        if ( !mpAnchoredDrawObj->ConsiderForTextWrap() )
        {
            // indicate that object has to be considered for text wrap
            mpAnchoredDrawObj->SetConsiderForTextWrap( true );
            // invalidate 'background' in order to allow its 'background'
            // to wrap around it.
            mpAnchoredDrawObj->NotifyBackground( mpAnchoredDrawObj->GetPageFrm(),
                                    mpAnchoredDrawObj->GetObjRectWithSpaces(),
                                    PREP_FLY_ARRIVE );
            // invalidate position of anchor frame in order to force
            // a re-format of the anchor frame, which also causes a
            // re-format of the invalid previous frames of the anchor frame.
            mpAnchoredDrawObj->AnchorFrm()->InvalidatePos();
        }
    }
}

// --> OD 2004-08-12 #i32795#
Point SwPosNotify::LastObjPos() const
{
    return maOldObjRect.Pos();
}
//<--

// ============================================================================
// OD 2004-08-12 #i32795#
// helper class for oscillation control on object positioning
// ============================================================================
class SwObjPosOscillationControl
{
    private:
        sal_uInt8 mnPosStackSize;

        const SwAnchoredDrawObject* mpAnchoredDrawObj;

        std::vector<Point*> maObjPositions;

    public:
        SwObjPosOscillationControl( const SwAnchoredDrawObject& _rAnchoredDrawObj );
        ~SwObjPosOscillationControl();

        bool OscillationDetected();
};

SwObjPosOscillationControl::SwObjPosOscillationControl(
                                const SwAnchoredDrawObject& _rAnchoredDrawObj )
    : mnPosStackSize( 20 ),
      mpAnchoredDrawObj( &_rAnchoredDrawObj )
{
}

SwObjPosOscillationControl::~SwObjPosOscillationControl()
{
    while ( !maObjPositions.empty() )
    {
        Point* pPos = maObjPositions.back();
        delete pPos;

        maObjPositions.pop_back();
    }
}

bool SwObjPosOscillationControl::OscillationDetected()
{
    bool bOscillationDetected = false;

    if ( maObjPositions.size() == mnPosStackSize )
    {
        // position stack is full -> oscillation
        bOscillationDetected = true;
    }
    else
    {
        Point* pNewObjPos = new Point( mpAnchoredDrawObj->GetObjRect().Pos() );
        for ( std::vector<Point*>::iterator aObjPosIter = maObjPositions.begin();
              aObjPosIter != maObjPositions.end();
              ++aObjPosIter )
        {
            if ( *(pNewObjPos) == *(*aObjPosIter) )
            {
                // position already occured -> oscillation
                bOscillationDetected = true;
                delete pNewObjPos;
                break;
            }
        }
        if ( !bOscillationDetected )
        {
            maObjPositions.push_back( pNewObjPos );
        }
    }

    return bOscillationDetected;
}

// ============================================================================
// implementation of class <SwAnchoredDrawObject>
// ============================================================================
TYPEINIT1(SwAnchoredDrawObject,SwAnchoredObject);

SwAnchoredDrawObject::SwAnchoredDrawObject() :
    SwAnchoredObject(),
    mbValidPos( false ),
    // --> OD 2004-09-29 #i34748#
    mpLastObjRect( 0L ),
    // <--
    mbNotYetAttachedToAnchorFrame( true ),
    // --> OD 2004-08-09 #i28749#
    mbNotYetPositioned( true ),
    // <--
    // --> OD 2006-03-17 #i62875#
    mbCaptureAfterLayoutDirChange( false )
    // <--
{
}

SwAnchoredDrawObject::~SwAnchoredDrawObject()
{
    // --> OD 2004-11-03 - follow-up of #i34748#
    delete mpLastObjRect;
    // <--
}

// --> OD 2006-03-17 #i62875#
void SwAnchoredDrawObject::UpdateLayoutDir()
{
    SwFrmFmt::tLayoutDir nOldLayoutDir( GetFrmFmt().GetLayoutDir() );

    SwAnchoredObject::UpdateLayoutDir();

    if ( !NotYetPositioned() &&
         GetFrmFmt().GetLayoutDir() != nOldLayoutDir &&
         GetFrmFmt().GetDoc()->get(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE) &&
         !IsOutsidePage() )
    {
        mbCaptureAfterLayoutDirChange = true;
    }
}
// <--

// --> OD 2006-03-17 #i62875#
bool SwAnchoredDrawObject::IsOutsidePage() const
{
    bool bOutsidePage( false );

    if ( !NotYetPositioned() && GetPageFrm() )
    {
        SwRect aTmpRect( GetObjRect() );
        bOutsidePage =
            ( aTmpRect.Intersection( GetPageFrm()->Frm() ) != GetObjRect() );
    }

    return bOutsidePage;
}
// <--

// =============================================================================
// OD 2004-03-25 #i26791# - implementation of pure virtual method declared in
// base class <SwAnchoredObject>
// =============================================================================
void SwAnchoredDrawObject::MakeObjPos()
{
    if ( IsPositioningInProgress() )
    {
        // nothind to do - positioning already in progress
        return;
    }

    if ( mbValidPos )
    {
        // nothing to do - position is valid
        return;
    }

    // --> OD 2004-08-09 #i28749# - anchored drawing object has to be attached
    // to anchor frame
    if ( mbNotYetAttachedToAnchorFrame )
    {
        ASSERT( false,
                "<SwAnchoredDrawObject::MakeObjPos() - drawing object not yet attached to anchor frame -> no positioning" );
        return;
    }

    SwDrawContact* pDrawContact =
                        static_cast<SwDrawContact*>(::GetUserCall( GetDrawObj() ));

    // --> OD 2004-08-09 #i28749# - if anchored drawing object hasn't been yet
    // positioned, convert its positioning attributes, if its positioning
    // attributes are given in horizontal left-to-right layout.
    // --> OD 2004-10-25 #i36010# - Note: horizontal left-to-right layout is made
    // the default layout direction for <SwDrawFrmFmt> instances. Thus, it has
    // to be adjusted manually, if no adjustment of the positioning attributes
    // have to be performed here.
    // --> OD 2004-11-17 #i35635# - additionally move drawing object to the
    // visible layer.
    if ( mbNotYetPositioned )
    {
        // --> OD 2004-11-17 #i35635#
        pDrawContact->MoveObjToVisibleLayer( DrawObj() );
        // <--
        // --> OD 2004-09-29 #117975# - perform conversion of positioning
        // attributes only for 'master' drawing objects
        // --> OD 2005-03-11 #i44334#, #i44681# - check, if positioning
        // attributes already have been set.
        if ( !GetDrawObj()->ISA(SwDrawVirtObj) &&
             !static_cast<SwDrawFrmFmt&>(GetFrmFmt()).IsPosAttrSet() )
        {
            _SetPositioningAttr();
        }
        // <--
        // --> OD 2006-05-24 #b6418964#
        // - reset internal flag after all needed actions are performed to
        //   avoid callbacks from drawing layer
        mbNotYetPositioned = false;
        // <--
    }
    // <--

    // indicate that positioning is in progress
    {
        SwObjPositioningInProgress aObjPosInProgress( *this );

        // determine relative position of drawing object and set it
        switch ( pDrawContact->GetAnchorId() )
        {
            case FLY_AS_CHAR:
            {
                // indicate that position will be valid after positioning is performed
                mbValidPos = true;
                // nothing to do, because as-character anchored objects are positioned
                // during the format of its anchor frame - see <SwFlyCntPortion::SetBase(..)>
            }
            break;
            case FLY_AT_PARA:
            case FLY_AT_CHAR:
            {
                // --> OD 2004-08-12 #i32795# - move intrinsic positioning to
                // helper method <_MakeObjPosAnchoredAtPara()>
                _MakeObjPosAnchoredAtPara();
            }
            break;
            case FLY_AT_PAGE:
            case FLY_AT_FLY:
            {
                // --> OD 2004-08-12 #i32795# - move intrinsic positioning to
                // helper method <_MakeObjPosAnchoredAtLayout()>
                _MakeObjPosAnchoredAtLayout();
            }
            break;
            default:
            {
                ASSERT( false, "<SwAnchoredDrawObject::MakeObjPos()> - unknown anchor type - please inform OD." );
            }
        }

        // keep, current object rectangle
        // --> OD 2004-09-29 #i34748# - use new method <SetLastObjRect(..)>
        SetLastObjRect( GetObjRect().SVRect() );
        // <--

        // Assure for 'master' drawing object, that it's registered at the correct page.
        // Perform check not for as-character anchored drawing objects and only if
        // the anchor frame is valid.
        if ( !GetDrawObj()->ISA(SwDrawVirtObj) &&
             !pDrawContact->ObjAnchoredAsChar() &&
             GetAnchorFrm()->IsValid() )
        {
            pDrawContact->ChkPage();
        }
    }

    // --> OD 2006-03-17 #i62875#
    if ( mbCaptureAfterLayoutDirChange &&
         GetPageFrm() )
    {
        SwRect aPageRect( GetPageFrm()->Frm() );
        SwRect aObjRect( GetObjRect() );
        if ( aObjRect.Right() >= aPageRect.Right() + 10 )
        {
            Size aSize( aPageRect.Right() - aObjRect.Right(), 0 );
            DrawObj()->Move( aSize );
            aObjRect = GetObjRect();
        }

        if ( aObjRect.Left() + 10 <= aPageRect.Left() )
        {
            Size aSize( aPageRect.Left() - aObjRect.Left(), 0 );
            DrawObj()->Move( aSize );
        }

        mbCaptureAfterLayoutDirChange = false;
    }
    // <--
}

/** method for the intrinsic positioning of a at-paragraph|at-character
    anchored drawing object

    OD 2004-08-12 #i32795# - helper method for method <MakeObjPos>

    @author OD
*/
void SwAnchoredDrawObject::_MakeObjPosAnchoredAtPara()
{
    // --> OD 2004-08-12 #i32795# - adopt positioning algorithm from Writer
    // fly frames, which are anchored at paragraph|at character

    // Determine, if anchor frame can/has to be formatted.
    // If yes, after each object positioning the anchor frame is formatted.
    // If after the anchor frame format the object position isn't valid, the
    // object is positioned again.
    // --> OD 2005-02-22 #i43255# - refine condition: anchor frame format not
    // allowed, if another anchored object, has to be consider its wrap influence
    // --> OD 2005-06-07 #i50356# - format anchor frame containing the anchor
    // position. E.g., for at-character anchored object this can be the follow
    // frame of the anchor frame, which contains the anchor character.
    const bool bFormatAnchor =
            !static_cast<const SwTxtFrm*>( GetAnchorFrmContainingAnchPos() )->IsAnyJoinLocked() &&
            !ConsiderObjWrapInfluenceOnObjPos() &&
            !ConsiderObjWrapInfluenceOfOtherObjs();
    // <--

    if ( bFormatAnchor )
    {
        // --> OD 2005-06-07 #i50356#
        GetAnchorFrmContainingAnchPos()->Calc();
        // <--
    }

    bool bOscillationDetected = false;
    SwObjPosOscillationControl aObjPosOscCtrl( *this );
    // --> OD 2004-08-25 #i3317# - boolean, to apply temporarly the
    // 'straightforward positioning process' for the frame due to its
    // overlapping with a previous column.
    bool bConsiderWrapInfluenceDueToOverlapPrevCol( false );
    // <--
    do {
        // indicate that position will be valid after positioning is performed
        mbValidPos = true;

        // --> OD 2004-10-20 #i35640# - correct scope for <SwPosNotify> instance
        {
            // create instance of <SwPosNotify> for correct notification
            SwPosNotify aPosNotify( this );

            // determine and set position
            objectpositioning::SwToCntntAnchoredObjectPosition
                    aObjPositioning( *DrawObj() );
            aObjPositioning.CalcPosition();

            // get further needed results of the positioning algorithm
            SetVertPosOrientFrm ( aObjPositioning.GetVertPosOrientFrm() );
            _SetDrawObjAnchor();

            // check for object position oscillation, if position has changed.
            if ( GetObjRect().Pos() != aPosNotify.LastObjPos() )
            {
                bOscillationDetected = aObjPosOscCtrl.OscillationDetected();
            }
        }
        // <--
        // format anchor frame, if requested.
        // Note: the format of the anchor frame can cause the object position
        // to be invalid.
        if ( bFormatAnchor )
        {
            // --> OD 2005-06-07 #i50356#
            GetAnchorFrmContainingAnchPos()->Calc();
            // <--
        }

        // --> OD 2004-08-25 #i3317#
        if ( !ConsiderObjWrapInfluenceOnObjPos() &&
             OverlapsPrevColumn() )
        {
            bConsiderWrapInfluenceDueToOverlapPrevCol = true;
        }
        // <--
    } while ( !mbValidPos && !bOscillationDetected &&
              !bConsiderWrapInfluenceDueToOverlapPrevCol );

    // --> OD 2004-08-25 #i3317# - consider a detected oscillation and overlapping
    // with previous column.
    // temporarly consider the anchored objects wrapping style influence
    if ( bOscillationDetected || bConsiderWrapInfluenceDueToOverlapPrevCol )
    {
        SetTmpConsiderWrapInfluence( true );
        SetRestartLayoutProcess( true );
    }
    // <--
}

/** method for the intrinsic positioning of a at-page|at-frame anchored
    drawing object

    OD 2004-08-12 #i32795# - helper method for method <MakeObjPos>

    @author OD
*/
void SwAnchoredDrawObject::_MakeObjPosAnchoredAtLayout()
{
    // indicate that position will be valid after positioning is performed
    mbValidPos = true;

    // create instance of <SwPosNotify> for correct notification
    SwPosNotify aPosNotify( this );

    // determine position
    objectpositioning::SwToLayoutAnchoredObjectPosition
            aObjPositioning( *DrawObj() );
    aObjPositioning.CalcPosition();

    // set position

    // --> OD 2004-07-29 #i31698#
    // --> OD 2004-10-18 #i34995# - setting anchor position needed for filters,
    // especially for the xml-filter to the OpenOffice.org file format
    {
        const Point aNewAnchorPos =
                    GetAnchorFrm()->GetFrmAnchorPos( ::HasWrap( GetDrawObj() ) );
        DrawObj()->SetAnchorPos( aNewAnchorPos );
        // --> OD 2006-10-05 #i70122# - missing invalidation
        InvalidateObjRectWithSpaces();
        // <--
    }
    // <--
    SetCurrRelPos( aObjPositioning.GetRelPos() );
    const SwFrm* pAnchorFrm = GetAnchorFrm();
    SWRECTFN( pAnchorFrm );
    const Point aAnchPos( (pAnchorFrm->Frm().*fnRect->fnGetPos)() );
    SetObjLeft( aAnchPos.X() + GetCurrRelPos().X() );
    SetObjTop( aAnchPos.Y() + GetCurrRelPos().Y() );
    // <--
}

void SwAnchoredDrawObject::_SetDrawObjAnchor()
{
    // new anchor position
    // --> OD 2004-07-29 #i31698# -
    Point aNewAnchorPos =
                GetAnchorFrm()->GetFrmAnchorPos( ::HasWrap( GetDrawObj() ) );
    Point aCurrAnchorPos = GetDrawObj()->GetAnchorPos();
    if ( aNewAnchorPos != aCurrAnchorPos )
    {
        // determine movement to be applied after setting the new anchor position
        Size aMove( aCurrAnchorPos.X() - aNewAnchorPos.X(),
                    aCurrAnchorPos.Y() - aNewAnchorPos.Y() );
        // set new anchor position
        DrawObj()->SetAnchorPos( aNewAnchorPos );
        // correct object position, caused by setting new anchor position
        DrawObj()->Move( aMove );
        // --> OD 2006-10-05 #i70122# - missing invalidation
        InvalidateObjRectWithSpaces();
        // <--
    }
}

/** method to invalidate the given page frame

    OD 2004-07-02 #i28701#

    @author OD
*/
void SwAnchoredDrawObject::_InvalidatePage( SwPageFrm* _pPageFrm )
{
    if ( _pPageFrm && !_pPageFrm->GetFmt()->GetDoc()->IsInDtor() )
    {
        if ( _pPageFrm->GetUpper() )
        {
            // --> OD 2004-11-11 #i35007# - correct invalidation for as-character
            // anchored objects.
            if ( GetFrmFmt().GetAnchor().GetAnchorId() == FLY_AS_CHAR )
            {
                _pPageFrm->InvalidateFlyInCnt();
            }
            else
            {
                _pPageFrm->InvalidateFlyLayout();
            }
            // <--

            SwRootFrm* pRootFrm = static_cast<SwRootFrm*>(_pPageFrm->GetUpper());
            pRootFrm->DisallowTurbo();
            if ( pRootFrm->GetTurbo() )
            {
                const SwCntntFrm* pTmpFrm = pRootFrm->GetTurbo();
                pRootFrm->ResetTurbo();
                pTmpFrm->InvalidatePage();
            }
            pRootFrm->SetIdleFlags();
        }
    }
}

void SwAnchoredDrawObject::InvalidateObjPos()
{
    // --> OD 2004-07-01 #i28701# - check, if invalidation is allowed
    if ( mbValidPos &&
         InvalidationOfPosAllowed() )
    {
        mbValidPos = false;
        // --> OD 2006-08-10 #i68520#
        InvalidateObjRectWithSpaces();
        // <--

        // --> OD 2005-03-08 #i44339# - check, if anchor frame exists.
        if ( GetAnchorFrm() )
        {
            // --> OD 2004-11-22 #118547# - notify anchor frame of as-character
            // anchored object, because its positioned by the format of its anchor frame.
            // --> OD 2005-03-09 #i44559# - assure, that text hint is already
            // existing in the text frame
            if ( GetAnchorFrm()->ISA(SwTxtFrm) &&
                 (GetFrmFmt().GetAnchor().GetAnchorId() == FLY_AS_CHAR) )
            {
                SwTxtFrm* pAnchorTxtFrm( static_cast<SwTxtFrm*>(AnchorFrm()) );
                if ( pAnchorTxtFrm->GetTxtNode()->GetpSwpHints() &&
                     pAnchorTxtFrm->CalcFlyPos( &GetFrmFmt() ) != STRING_LEN )
                {
                    AnchorFrm()->Prepare( PREP_FLY_ATTR_CHG, &GetFrmFmt() );
                }
            }
            // <--

            SwPageFrm* pPageFrm = AnchorFrm()->FindPageFrm();
            _InvalidatePage( pPageFrm );

            // --> OD 2004-08-12 #i32270# - also invalidate page frame, at which the
            // drawing object is registered at.
            SwPageFrm* pPageFrmRegisteredAt = GetPageFrm();
            if ( pPageFrmRegisteredAt &&
                 pPageFrmRegisteredAt != pPageFrm )
            {
                _InvalidatePage( pPageFrmRegisteredAt );
            }
            // <--
            // --> OD 2004-09-23 #i33751#, #i34060# - method <GetPageFrmOfAnchor()>
            // is replaced by method <FindPageFrmOfAnchor()>. It's return value
            // have to be checked.
            SwPageFrm* pPageFrmOfAnchor = FindPageFrmOfAnchor();
            if ( pPageFrmOfAnchor &&
                 pPageFrmOfAnchor != pPageFrm &&
                 pPageFrmOfAnchor != pPageFrmRegisteredAt )
            // <--
            {
                _InvalidatePage( pPageFrmOfAnchor );
            }
        }
        // <--
    }
}

SwFrmFmt& SwAnchoredDrawObject::GetFrmFmt()
{
    ASSERT( static_cast<SwDrawContact*>(GetUserCall(GetDrawObj()))->GetFmt(),
            "<SwAnchoredDrawObject::GetFrmFmt()> - missing frame format -> crash." );
    return *(static_cast<SwDrawContact*>(GetUserCall(GetDrawObj()))->GetFmt());
}
const SwFrmFmt& SwAnchoredDrawObject::GetFrmFmt() const
{
    ASSERT( static_cast<SwDrawContact*>(GetUserCall(GetDrawObj()))->GetFmt(),
            "<SwAnchoredDrawObject::GetFrmFmt()> - missing frame format -> crash." );
    return *(static_cast<SwDrawContact*>(GetUserCall(GetDrawObj()))->GetFmt());
}

const SwRect SwAnchoredDrawObject::GetObjRect() const
{
    // use geometry of drawing object
    //return GetDrawObj()->GetCurrentBoundRect();
    return GetDrawObj()->GetSnapRect();
}

// --> OD 2006-10-05 #i70122#
const SwRect SwAnchoredDrawObject::GetObjBoundRect() const
{
    return GetDrawObj()->GetCurrentBoundRect();
}
// <--

// --> OD 2006-08-10 #i68520#
bool SwAnchoredDrawObject::_SetObjTop( const SwTwips _nTop )
{
    SwTwips nDiff = _nTop - GetObjRect().Top();
    DrawObj()->Move( Size( 0, nDiff ) );

    return nDiff != 0;
}
bool SwAnchoredDrawObject::_SetObjLeft( const SwTwips _nLeft )
{
    SwTwips nDiff = _nLeft - GetObjRect().Left();
    DrawObj()->Move( Size( nDiff, 0 ) );

    return nDiff != 0;
}
// <--

/** adjust positioning and alignment attributes for new anchor frame

    OD 2004-08-24 #i33313# - add second optional parameter <_pNewObjRect>

    @author OD
*/
void SwAnchoredDrawObject::AdjustPositioningAttr( const SwFrm* _pNewAnchorFrm,
                                                  const SwRect* _pNewObjRect )
{
    SwTwips nHoriRelPos = 0;
    SwTwips nVertRelPos = 0;
    const Point aAnchorPos = _pNewAnchorFrm->GetFrmAnchorPos( ::HasWrap( GetDrawObj() ) );
    // --> OD 2004-08-24 #i33313#
    const SwRect aObjRect( _pNewObjRect ? *_pNewObjRect : GetObjRect() );
    // <--
    const bool bVert = _pNewAnchorFrm->IsVertical();
    const bool bR2L = _pNewAnchorFrm->IsRightToLeft();
    if ( bVert )
    {
        nHoriRelPos = aObjRect.Top() - aAnchorPos.Y();
        nVertRelPos = aAnchorPos.X() - aObjRect.Right();
    }
    else if ( bR2L )
    {
        nHoriRelPos = aAnchorPos.X() - aObjRect.Right();
        nVertRelPos = aObjRect.Top() - aAnchorPos.Y();
    }
    else
    {
        nHoriRelPos = aObjRect.Left() - aAnchorPos.X();
        nVertRelPos = aObjRect.Top() - aAnchorPos.Y();
    }

    GetFrmFmt().SetFmtAttr( SwFmtHoriOrient( nHoriRelPos, text::HoriOrientation::NONE, text::RelOrientation::FRAME ) );
    GetFrmFmt().SetFmtAttr( SwFmtVertOrient( nVertRelPos, text::VertOrientation::NONE, text::RelOrientation::FRAME ) );
}

// --> OD 2004-09-29 #i34748# - change return type
const Rectangle* SwAnchoredDrawObject::GetLastObjRect() const
{
    return mpLastObjRect;
}
// <--

// --> OD 2004-09-29 #i34748# - change return type.
// If member <mpLastObjRect> is NULL, create one.
void SwAnchoredDrawObject::SetLastObjRect( const Rectangle& _rNewLastRect )
{
    if ( !mpLastObjRect )
    {
        mpLastObjRect = new Rectangle;
    }
    *(mpLastObjRect) = _rNewLastRect;
}
// <--

void SwAnchoredDrawObject::ObjectAttachedToAnchorFrame()
{
    // --> OD 2004-07-27 #i31698#
    SwAnchoredObject::ObjectAttachedToAnchorFrame();
    // <--

    if ( mbNotYetAttachedToAnchorFrame )
    {
        mbNotYetAttachedToAnchorFrame = false;
    }
}

/** method to set positioning attributes

    OD 2004-10-20 #i35798#
    During load the positioning attributes aren't set.
    Thus, the positioning attributes are set by the current object geometry.
    This method is also used for the conversion for drawing objects
    (not anchored as-character) imported from OpenOffice.org file format
    once and directly before the first positioning.

    @author OD
*/
void SwAnchoredDrawObject::_SetPositioningAttr()
{
    SwDrawContact* pDrawContact =
                        static_cast<SwDrawContact*>(GetUserCall( GetDrawObj() ));

    if ( !pDrawContact->ObjAnchoredAsChar() )
    {
        SwRect aObjRect( GetObjRect() );

        SwTwips nHoriPos = aObjRect.Left();
        SwTwips nVertPos = aObjRect.Top();
        // --> OD 2005-03-10 #i44334#, #i44681#
        // perform conversion only if position is in horizontal-left-to-right-layout.
        if ( GetFrmFmt().GetPositionLayoutDir() ==
                text::PositionLayoutDir::PositionInHoriL2R )
        {
            SwFrmFmt::tLayoutDir eLayoutDir = GetFrmFmt().GetLayoutDir();
            switch ( eLayoutDir )
            {
                case SwFrmFmt::HORI_L2R:
                {
                    // nothing to do
                }
                break;
                case SwFrmFmt::HORI_R2L:
                {
                    nHoriPos = -aObjRect.Left() - aObjRect.Width();
                }
                break;
                case SwFrmFmt::VERT_R2L:
                {
                    nHoriPos = aObjRect.Top();
                    nVertPos = -aObjRect.Left() - aObjRect.Width();
                }
                break;
                default:
                {
                    ASSERT( false,
                            "<SwAnchoredDrawObject::_SetPositioningAttr()> - unsupported layout direction" );
                }
            }
        }
        // <--

        // --> OD 2006-11-10 #i71182#
        // only change position - do not lose other attributes
        SwFmtHoriOrient aHori( GetFrmFmt().GetHoriOrient() );
        aHori.SetPos( nHoriPos );
        GetFrmFmt().SetFmtAttr( aHori );

        SwFmtVertOrient aVert( GetFrmFmt().GetVertOrient() );
        // --> OD 2007-01-03 #i73079# - vertical position already correct
//        if ( aVert.GetRelationOrient() == text::RelOrientation::CHAR ||
//             aVert.GetRelationOrient() == text::RelOrientation::TEXT_LINE )
//        {
//            nVertPos = -nVertPos;
//        }
        // <--
        aVert.SetPos( nVertPos );
        GetFrmFmt().SetFmtAttr( aVert );
        // <--

        // --> OD 2004-10-25 #i36010# - set layout direction of the position
        GetFrmFmt().SetPositionLayoutDir(
            text::PositionLayoutDir::PositionInLayoutDirOfAnchor );
        // <--
    }
    // --> OD 2007-11-29 #i65798# - also for as-character anchored objects
    // --> OD 2005-05-10 #i45952# - indicate that position
    // attributes are set now.
    static_cast<SwDrawFrmFmt&>(GetFrmFmt()).PosAttrSet();
    // <--
}

void SwAnchoredDrawObject::NotifyBackground( SwPageFrm* _pPageFrm,
                                             const SwRect& _rRect,
                                             PrepareHint _eHint )
{
    ::Notify_Background( GetDrawObj(), _pPageFrm, _rRect, _eHint, sal_True );
}

/** method to assure that anchored object is registered at the correct
    page frame

    OD 2004-07-02 #i28701#

    @author OD
*/
void SwAnchoredDrawObject::RegisterAtCorrectPage()
{
    SwPageFrm* pPageFrm( 0L );
    if ( GetVertPosOrientFrm() )
    {
        pPageFrm = const_cast<SwPageFrm*>(GetVertPosOrientFrm()->FindPageFrm());
    }
    if ( pPageFrm && GetPageFrm() != pPageFrm )
    {
        if ( GetPageFrm() )
            GetPageFrm()->RemoveDrawObjFromPage( *this );
        pPageFrm->AppendDrawObjToPage( *this );
    }
}

// =============================================================================
