(define (phseq0 segs)
  (cond
   ((null? segs)
    '("pau"))
   ((equal? (item.feat (car segs) "R:SylStructure.parent.parent.name") 0)
    (phseq0 (cdr segs)))
   ((and
     (equal? (item.feat (car segs) "pos_in_syl") 0)
     (equal? (item.feat (car segs) "R:SylStructure.parent.pos_in_word") 0))
    (cons
     (if (equal? (item.feat (car segs) "R:SylStructure.parent.syl_in") 0) "pau" "ssil")
     (cons (item.name (car segs)) (phseq0 (cdr segs)))))
   (t
    (cons (item.name (car segs)) (phseq0 (cdr segs))))))

(define (phseq path)
  (phseq0 (utt.relation.items (utt.load nil path) 'Segment)))

(define (print-phseq path)
  (mapcar
   (lambda (ph)
     (format t "%s " ph))
   (phseq path))
  (format t "\n"))

(define (align old_segs new_segs)
  (cond
   ((null? old_segs)
    t)
   ((null? new_segs)
    (item.delete (car old_segs))
(align (cdr old_segs) new_segs))
   ((equal? (item.name (car old_segs)) (item.name (car new_segs)))
    (item.set_feat (car old_segs) 'end (item.feat (car new_segs) 'end))
    (align (cdr old_segs) (cdr new_segs)))
   ((equal? (item.name (car old_segs)) "pau")
    (item.delete (car old_segs))
    (align (cdr old_segs) new_segs))
   ((equal? (item.name (car new_segs)) "pau")
    (item.insert (car old_segs) (car new_segs) 'before)
    (align old_segs (cdr new_segs)))
   (t
    (format stderr "%s@%f!=%s\n" (item.name (car new_segs)) (item.feat (car new_segs) 'end) (item.name (car old_segs)))
    (exit 1))))

(define (split utt words)
  (if (null? words)
      t
      (begin
        (if (string-equal (item.feat (car words) "R:SylStructure.daughter1.daughter1.R:Segment.p.name") "pau")
            (utt.relation.append utt 'Phrase nil))
        (item.append_daughter (utt.relation.last utt 'Phrase) (car words))
        (split utt (cdr words)))))

(define (merge-labs inpath labpath outpath)
  (set! utt (utt.load nil inpath))
  (utt.relation.load utt 'NewSegment labpath)
  (align (utt.relation.items utt 'Segment) (utt.relation.items utt 'NewSegment))
  (utt.relation.delete utt 'Phrase)
  (utt.relation.create utt 'Phrase)
  (split utt (utt.relation.items utt 'Word))
  (utt.save utt outpath nil))
