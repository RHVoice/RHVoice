;; These rules are used in most cases
;; when there are no vowels in the word.

(lts.ruleset
 ru_consonants
 ()
 (
  ( # [ л ] # = ee ll )
  ( # [ l ] # = ee ll )
  ( [ б ] # = b ee )
  ( [ б ] = b e )
  ( [ в ] # = v ee )
  ( [ в ] = v e )
  ( [ г ] # = g ee )
  ( [ г ] = g e )
  ( [ д ] # = d ee )
  ( [ д ] = d e )
  ( [ ж ] # = zh ee )
  ( [ ж ] = zh e )
  ( [ з ] # = z ee )
  ( [ з ] = z e )
  ( [ к ] # = k aa )
  ( [ к ] = k a )
  ( [ л ] # = ee l )
  ( [ л ] = e l )
  ( [ м ] # = ee m )
  ( [ м ] = e m )
  ( [ н ] # = ee n )
  ( [ н ] = e n )
  ( [ п ] # = p ee )
  ( [ п ] = p e )
  ( [ р ] # = ee r )
  ( [ р ] = e r )
  ( [ с ] # = ee s )
  ( [ с ] = e s )
  ( [ т ] # = t ee )
  ( [ т ] = t e )
  ( [ ф ] # = ee f )
  ( [ ф ] = e f )
  ( [ х ] # = h aa )
  ( [ х ] = h a )
  ( [ ц ] # = c ee )
  ( [ ц ] = c e )
  ( [ ч ] # = ch ee )
  ( [ ч ] = ch e )
  ( [ ш ] # = sh aa )
  ( [ ш ] = sh a )
  ( [ щ ] # = sch aa )
  ( [ щ ] = sch a )
  ( [ b ] # = b ee )
  ( [ b ] = b e )
  ( [ c ] # = c ee )
  ( [ c ] = sc e )
  ( [ d ] # = d ee )
  ( [ d ] = d e )
  ( [ f ] # = ee f )
  ( [ f ] = e f )
  ( [ g ] # = g ee )
  ( [ g ] = g e )
  ( [ h ] # = h aa )
  ( [ h ] = h a )
  ( [ j ] # = j ee )
  ( [ j ] = j e )
  ( [ k ] # = k aa )
  ( [ k ] = k a )
  ( [ l ] # = ee l )
  ( [ l ] = e l )
  ( [ m ] # = ee m )
  ( [ m ] = e m )
  ( [ n ] # = ee n )
  ( [ n ] = e n )
  ( [ p ] # = p ee )
  ( [ p ] = p e )
  ( [ q ] # = k uu )
  ( [ q ] = k u )
  ( [ r ] # = ee r )
  ( [ r ] = e r )
  ( [ s ] # = ee s )
  ( [ s ] = e s )
  ( [ t ] # = t ee )
  ( [ t ] = t e )
  ( [ v ] # = v ee )
  ( [ v ] = v e )
  ( [ w ] = d u b ll v ee )
  ( [ x ] # = ii k s )
  ( [ x ] = i k s )
  ( [ y ] = ii g rr e k )
  ( [ z ] # = z ee d )
  ( [ z ] = z e d )
  ( [ + ] = )
  ( [ - ] = )
  ( [ "'" ] = )
  ))

(provide 'ru_consonants_lts)
