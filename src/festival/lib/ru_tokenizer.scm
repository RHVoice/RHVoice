(require 'ru_digits_lts)
(require 'ru_number_lts)
(require 'ru_downcase_lts)

(set! russian_token_punctuation "\"'`.,:;!?-(){}[]")
(set! russian_token_prepunctuation "\"'`({[")
(set! russian_token_whitespace " \t\n\r")
(set! russian_token_singlecharsymbols "")

(define (ru::token_to_words token name)
  "(ru::token_to_words token name)
Specific token to word rules for the russian language.  Returns a list
of words that expand given token with name."
  (cond
   ((string-matches name "[0-9]+")
    (russian_digits_to_word name))
   (t
    (russian_letters_to_word name))))

(define (russian_letters_to_word str)
  (let
      ((l) (s) (s1) (s2))
    (set! s (russian_downcase str))
    (set! l nil)
    (while
     s
     (set! s2 (string-after s "|"))
     (if (string-equal s2 "")
         (set! s2 nil))
     (set! s1 (string-before s "|"))
     (if (string-equal s1 "")
         (set! s1 s))
     (set! l (cons s1 l))
     (set! s s2))
    (reverse l)))

;; I'm using lts for numbers,
;; because they can be converted to Flite format automaticly

(define (russian_digits_to_word str)
  (let
      ((digits (symbolexplode str)))
    (cond
     ((or
       (string-equal (car digits) "0")
       (> (length digits) 15))
      (lts.apply digits 'ru_digits))
     (t
      (lts.apply (russian_preprocess_number digits) 'ru_number)))))

;; convert a string of digits to an intermediate form for lts

(define (russian_preprocess_number x)
  (russian_preprocess_number_2 (reverse x) '() 0))

(define (russian_preprocess_number_2 x r n)
  (cond
   ((null? x) r)
   ((equal? n 3)
    (russian_preprocess_number_2 (cdr x) (cons (car x) (cons 'th r)) (+ n 1)))
   ((equal? n 6)
    (russian_preprocess_number_2 (cdr x) (cons (car x) (cons 'm r)) (+ n 1)))
   ((equal? n 9)
    (russian_preprocess_number_2 (cdr x) (cons (car x) (cons 'b r)) (+ n 1)))
   ((equal? n 12)
    (russian_preprocess_number_2 (cdr x) (cons (car x) (cons 'tr r)) (+ n 1)))
   (t (russian_preprocess_number_2 (cdr x) (cons (car x) r) (+ n 1)))))

(define (ru::select_tokenizer)
  "(ru::select_tokenizer)
Set up tokenizer for russian."
  (set! token.punctuation russian_token_punctuation)
  (set! token.prepunctuation russian_token_prepunctuation)
  (set! token.whitespace russian_token_whitespace)
  (set! token.singlecharsymbols russian_token_singlecharsymbols)
  (Parameter.set 'Language 'russian)
  (Parameter.set 'Token_Method 'Token_Any)
  (set! token_to_words ru::token_to_words))

(define (ru::reset_tokenizer)
  "(ru::reset_tokenizer)"
  t)

(provide 'ru_tokenizer)
