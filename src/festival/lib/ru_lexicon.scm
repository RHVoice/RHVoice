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
;;; Lexicon, LTS and Postlexical rules for russian

(require 'ru_lts)
(require 'ru_consonants_lts)
(require 'ru_vowel_reduction)
(require 'ru_consonant_vpairs)
(require 'ru_stress_tree)

(define (russian_is_vowel phone) 
  (member_string phone '(a e i o u y aa ee ii oo uu yy ae ay ur)))

(define (russian_is_sonorant phone) 
  (member_string phone '(l m n r ll mm nn rr)))

(define (russian_contains_vowel phones)
  (cond
   ((null phones) nil)
   ((russian_is_vowel (car phones))
    t)
   (t (russian_contains_vowel (cdr phones)))))

(define (russian_last_seg_in_rhyme utt)
  (russian_contains_vowel
   (mapcar
    item.name
    (item.daughters
     (item.relation.parent
      (utt.relation.last
       utt
       'Segment)
      'SylStructure)))))

(define (russian_last_seg_syl_final utt phones)
  (let (p n nn)
    (cond
     ((null? phones) t)
     (t
      (set! n (car phones))
      (cond
       ((equal? n 'pau) t)
       ((not (russian_contains_vowel phones)) nil)
       ((not (russian_last_seg_in_rhyme utt)) nil)
       ((russian_is_vowel n) t)
       ((null? (cdr phones)) nil)
       (t
        (set! p (item.name (utt.relation.last utt 'Segment)))
        (set! nn (car (cdr phones)))
        (cond
         ((russian_is_vowel p)
          (cond
           ((russian_is_vowel nn) t)
           ((and (russian_is_sonorant n) (not (russian_is_sonorant nn))) nil)
           ((equal? n 'j) nil)
           (t t)))
         ((russian_is_sonorant p) t)
         ((equal? p 'j) t)
         (t nil))))))))

(define (russian_word_to_phones word)
  (let
      ((letters) (n))
    (set! letters (utf8explode (item.name word)))
    (set! n (cadr (lex.lookup (item.name word))))
    (cond
     ((null? n) t)
     ((> n 0)
      (set-car! (nth_cdr (- n 1) letters) "ё"))
     (t
      (item.set_feat word 'stressed_syl_num n)))
    (cond
     ((and
       (lts.in.alphabet letters 'ru_consonants)
       (string-equal (item.feat word 'gpos) 'content))
      (item.set_feat word 'no_vr "1")
      (lts.apply letters 'ru_consonants))
     (t
      (lts.apply letters 'msu_ru)))))

(define (russian_word_function utt word)
  (let
      ((phones (russian_word_to_phones word)))
    (cond
     ((null? phones)
      (let
          ((phrase (item.relation.parent word 'Phrase)))
        (item.delete word)
        (if (null? (item.daughter1 phrase))
            (item.delete phrase))))
     (t
      (utt.relation.append utt 'SylStructure word)
      (utt.relation.append utt 'Transcription word)
      (russian_syllabify utt word phones t)
      (ru_assign_stress word)))))

(define (russian_syllabify utt word phones sylstart)
  (cond
   ((null? phones) t)
   (t
    (if sylstart
        (begin
          (utt.relation.append utt 'Syllable '("syl"))
          (item.relation.append_daughter
           word
           'SylStructure
           (utt.relation.last utt 'Syllable))))
    (utt.relation.append utt 'Segment (list (car phones)))
    (item.relation.append_daughter
     word
     'Transcription
     (utt.relation.last utt 'Segment))
    (item.relation.append_daughter
     (utt.relation.last utt 'Syllable)
     'SylStructure
     (utt.relation.last utt 'Segment))
    (if (russian_is_vowel (car phones))
        (begin
          (utt.relation.append
           utt
           'SylVowel
           (utt.relation.last utt 'Syllable))
          (item.relation.append_daughter
           (utt.relation.last utt 'Syllable)
           'SylVowel
           (utt.relation.last utt 'Segment))))
    (russian_syllabify
     utt
     word
     (cdr phones)
     (russian_last_seg_syl_final utt (cdr phones))))))

(define (russian_word_method utt)
  (utt.relation.create utt 'Syllable)
  (utt.relation.create utt 'Segment)
  (utt.relation.create utt 'SylStructure)
  (utt.relation.create utt 'Transcription)
  (utt.relation.create utt 'SylVowel)
  (mapcar
   (lambda (word)
     (russian_word_function utt word))
   (utt.relation.items utt 'Word))
  utt)

(define (ru_assign_stress word)
  (let
      ((numsyls) (syl) (ssylnum) (seg) (stressed))
    (set! numsyls (item.feat word 'word_numsyls))
    (cond
     ((and
       (string-equal (item.feat word 'gpos) 'enc)
       (item.relation.prev word 'Phrase))
      t)
     ((and
       (string-equal (item.feat word 'gpos) 'proc)
       (item.relation.next word 'Phrase))
      t)
     ((not
       (ru_find_vowel_seg
        (item.relation.daughter1 word 'Transcription)
        (item.relation.daughtern word 'Transcription)))
      t)
     ((equal? numsyls 1)
      (item.set_feat
       (item.relation.daughter1 word 'SylStructure)
       'stress
       "1"))
     (t
      (set! ssylnum (item.feat word 'stressed_syl_num))
      (set! stressed nil)
      (set! syl (item.relation.daughter1 word 'SylStructure))
      (while
       syl
       (set! seg (item.relation.daughter1 syl 'SylVowel))
       (if (russian_is_stressed_vowel (item.name seg))
           (begin
             (set! stressed t)
             (item.set_feat syl 'stress "1")))
       (set! syl (item.next syl)))
      (cond
       (stressed
        (cond
         ((< ssylnum 0)
          (item.set_feat
           (nth
            (+ numsyls ssylnum)
            (item.relation.daughters word 'SylStructure))
           'stress
           "1"))
         (t t)))
       (t
        (if (equal? ssylnum 0)
            (begin
              (set! ssylnum (wagon_predict word ru_stress_tree))
              (cond
               ((>= (+ numsyls ssylnum) 0) t)
               ((<= numsyls 4)
                (set! ssylnum -2))
               ((<= numsyls 6)
                (set! ssylnum -3))
               (t (set! ssylnum -4)))))
        (item.set_feat
         (nth
          (+ numsyls ssylnum)
          (item.relation.daughters word 'SylStructure))
         'stress
         "1")))))))

(define (russian_is_stressed_vowel ph)
  (member_string ph '(aa oo uu ee ii yy)))

(define (ru_find_vowel_seg f l)
  (cond
   ((russian_is_vowel (item.name f)) f)
   ((string-equal
     (item.feat f 'id)
     (item.feat l 'id))
    nil)
   (t
    (ru_find_vowel_seg (item.next f) l))))

(define (russian_postlex_function utt)
  (ru_reduce_vowels utt)
  (ru_correct_consonants utt)
  utt)

(set! russian_old_word_method nil)
(lex.create "ru_dict")
(lex.set.phoneset "msu_ru")
(lex.set.compile.file (library_expand_filename "ru_dict"))
(lex.set.lts.method 'none)

(define (ru::select_lexicon)
  (set! russian_old_word_method (Param.get 'Word_Method))
  (Param.set 'Word_Method 'russian_word_method)
  (lex.select "ru_dict")
  (set! postlex_rules_hooks (list russian_postlex_function)))

(define (ru::reset_lexicon)
  (Param.set 'Word_Method russian_old_word_method))

(provide 'ru_lexicon)
