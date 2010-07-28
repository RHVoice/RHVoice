;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;                                                                     ;;;
;;;                     Carnegie Mellon University                      ;;;
;;;                  and Alan W Black and Kevin Lenzo                   ;;;
;;;                      Copyright (c) 1998-2000                        ;;;
;;;                        All Rights Reserved.                         ;;;
;;;                                                                     ;;;
;;; Permission is hereby granted, free of charge, to use and distribute ;;;
;;; this software and its documentation without restriction, including  ;;;
;;; without limitation the rights to use, copy, modify, merge, publish, ;;;
;;; distribute, sublicense, and/or sell copies of this work, and to     ;;;
;;; permit persons to whom this work is furnished to do so, subject to  ;;;
;;; the following conditions:                                           ;;;
;;;  1. The code must retain the above copyright notice, this list of   ;;;
;;;     conditions and the following disclaimer.                        ;;;
;;;  2. Any modifications must be clearly marked as such.               ;;;
;;;  3. Original authors' names are not deleted.                        ;;;
;;;  4. The authors' names are not used to endorse or promote products  ;;;
;;;     derived from this software without specific prior written       ;;;
;;;     permission.                                                     ;;;
;;;                                                                     ;;;
;;; CARNEGIE MELLON UNIVERSITY AND THE CONTRIBUTORS TO THIS WORK        ;;;
;;; DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING     ;;;
;;; ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT  ;;;
;;; SHALL CARNEGIE MELLON UNIVERSITY NOR THE CONTRIBUTORS BE LIABLE     ;;;
;;; FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES   ;;;
;;; WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN  ;;;
;;; AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,         ;;;
;;; ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF      ;;;
;;; THIS SOFTWARE.                                                      ;;;
;;;                                                                     ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;; The phoneset has been taken from msu_ru_nsh_clunits voice.
;;; Phonset for msu_ru, mostly based on work of Zaharov L.M. "Acoustic
;;; variation of sounds in Russian speech". M. 2004
;;;

(defPhoneSet
  msu_ru
  ;;;  Phone Features
  (;; vowel or consonant
   (vc + -)  
   ;; vowel length: short long dipthong schwa
   (vlng s l a 0)
   ;; vowel height: high mid low
   (vheight 1 2 3 4 5 0)
   ;; vowel frontness: front mid back
   (vfront 1 2 3 4 5 0)
   ;; lip rounding
   (vrnd + - 0)
   ;; consonant type: stop fricative affricative nasal liquid
   (ctype s f a n l 0)
   ;; place of articulation: labial alveolar palatal labio-dental
   ;;                         dental velar
   (cplace l a p b d v 0)
   ;; consonant voicing
   (cvox + - 0)
   ;; consonant softness
   (csoft + - 0)   )
  (
   (pau     -  0  0  0  0  0  0  0  0)  ;; silence ... 

   ;;; stressed vowels
   
   (ii      +  l  5  1  -  0  0  0  0)
   (yy      +  l  5  3  -  0  0  0  0)
   (uu      +  l  5  5  +  0  0  0  0)
   (ee      +  l  3  1  -  0  0  0  0)
   (oo      +  l  3  5  +  0  0  0  0)
   (aa      +  l  1  3  -  0  0  0  0)
   ;;; vowels in first level of reduction
   
   (a       +  s  2  4  -  0  0  0  0)   ;; a and o reduced
   (e       +  s  3  2  -  0  0  0  0)   ;; e in open unstressed syllable 
   (i       +  s  4  2  -  0  0  0  0)   ;; i reduced
   (y       +  s  4  3  -  0  0  0  0)   ;; y reduced
   (u       +  s  5  5  +  0  0  0  0)   ;; u unstressed

   ;;; vowels in second level of reduction
   
   (ae      +  a  3  3  -  0  0  0  0)   ;; second level after soft consonant
   (ay      +  a  3  3  -  0  0  0  0)   ;; second level after hard consonant
   (ur      +  a  4  4  +  0  0  0  0)   ;; reduced u

   ;;; consonants

   (p       -  0  0  0  0  s  l  -  -)
   (pp      -  0  0  0  0  s  l  -  +)
   (b       -  0  0  0  0  s  l  +  -)
   (bb      -  0  0  0  0  s  l  +  +)
   (t       -  0  0  0  0  s  d  -  -)
   (tt      -  0  0  0  0  s  d  -  +)
   (d       -  0  0  0  0  s  d  +  -)
   (dd      -  0  0  0  0  s  d  +  +)
   (k       -  0  0  0  0  s  p  -  -)
   (kk      -  0  0  0  0  s  p  -  +)
   (g       -  0  0  0  0  s  p  +  -)
   (gg      -  0  0  0  0  s  p  +  +)
   (c       -  0  0  0  0  a  d  -  -)
   (ch      -  0  0  0  0  a  a  -  +)
   (f       -  0  0  0  0  f  b  -  -)
   (ff      -  0  0  0  0  f  b  -  +)
   (v       -  0  0  0  0  f  b  +  -)
   (vv      -  0  0  0  0  f  b  +  +)
   (s       -  0  0  0  0  f  d  -  -)
   (ss      -  0  0  0  0  f  d  -  +)
   (z       -  0  0  0  0  f  d  +  -)
   (zz      -  0  0  0  0  f  d  +  +)
   (sh      -  0  0  0  0  f  a  -  -)
   (sch     -  0  0  0  0  f  p  -  +)
   (zh      -  0  0  0  0  f  a  +  -)
   (h       -  0  0  0  0  f  v  -  -)
   (hh      -  0  0  0  0  f  v  -  +)
   (m       -  0  0  0  0  n  l  +  -)
   (mm      -  0  0  0  0  n  l  +  +)
   (n       -  0  0  0  0  n  d  +  -)
   (nn      -  0  0  0  0  n  d  +  +)
   (l       -  0  0  0  0  l  d  +  -)
   (ll      -  0  0  0  0  l  d  +  +)
   (r       -  0  0  0  0  l  a  +  -)
   (rr      -  0  0  0  0  l  a  +  +)
   (j       -  0  0  0  0  f  p  +  +)
   )  
  )

(PhoneSet.silences '(pau))


(define (is_pau i)
  (if (phone_is_silence (item.name i))
      "1"
      "0"))

(define (ru::select_phoneset)
  "(ru::select_phoneset)
Set up phone set for a russian voice."
  (Parameter.set 'PhoneSet 'msu_ru)
  (PhoneSet.select 'msu_ru)
  )

(define (ru::reset_phoneset)
  "(ru::reset_phoneset)
Reset phone set for a russian voice."
  t
  )

(provide 'ru_phoneset)
