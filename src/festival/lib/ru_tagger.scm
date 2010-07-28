(set! ru_guess_pos 
      '(
        (proc
         "а"
         "без"
         "безо"
         "в"
         "во"
         "для"
         "до"
         "за"
         "и"
         "из"
         "изо"
         "из-под"
         "из-за"
         "к"
         "ко"
         "меж"
         "на"
         "над"
         "не"
         "ни"
         "о"
         "об"
         "обо"
         "от"
         "ото"
         "перед"
         "передо"
         "по"
         "под"
         "пред"
         "при"
         "про"
         "с"
         "со"
         "у"
         "через"
         "чтоб"
         )
        (enc
         "б"
         "бы"
         "-де"
         "ж"
         "же"
         "-ка"
         "ли"
         "-либо"
         "-нибудь"
         "-с"
         "-таки"
         "-тка"
         "-то"
         )
        )
      )

(define (ru::select_tagger)
  "(ru::select_tagger)
Set up the POS tagger for ru."
  (set! pos_lex_name nil)
  (set! guess_pos ru_guess_pos) 
  )

(define (ru::reset_tagger)
  "(ru::reset_tagger)
Reset tagging information."
  t
  )

(provide 'ru_tagger)
