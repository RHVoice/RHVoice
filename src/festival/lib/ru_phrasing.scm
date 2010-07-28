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
;;; Prosodic phrasing
;;;

(set! ru_phrase_cart_tree
      '(
        (n.name is 0)
        ((BB))
        ((R:Token.n.name is 0)
         ((R:Token.parent.lisp_ru_token_punc_minor_break is 1)
          ((B))
          ((R:Token.parent.lisp_ru_token_punc_major_break is 1)
           ((BB))
           ((R:Token.parent.n.name is -)
            ((B))
            ((R:Token.parent.n.name is --)
             ((B))
             ((R:Token.parent.n.name is "")
              ((R:Token.parent.n.prepunctuation is "-")
               ((B))
               ((R:Token.parent.n.prepunctuation is "--")
                ((BB))
                ((NB))))
              ((NB)))))))
         ((NB)))))

(define (ru::select_phrasing)
  "(ru::select_phrasing)
Set up the phrasing module for Russian."
  (set! phrase_cart_tree ru_phrase_cart_tree)
  (Parameter.set 'Phrase_Method 'cart_tree)
  (Param.set 'Phrasify_Method Classic_Phrasify)
  )

(define (ru::reset_phrasing)
  "(ru::reset_phrasing)
Reset phrasing information."
  t
  )

(define (ru_token_punc_minor_break token)
  (if (string-matches (item.feat token 'punc) ".*[-,:;]\"?")
      "1"
      "0"))

(define (ru_token_punc_major_break token)
  (if (string-matches (item.feat token 'punc) ".*[.?!]\"?")
      "1"
      "0"))

(provide 'ru_phrasing)
