(define (display-@-and-return cc) (display #\@) cc)
(define (display-*-and-return cc) (display #\*) cc)
(define (get-current-continuation)
  (call-with-current-continuation (lambda (c) c)))
(let ((yin (display-@-and-return (get-current-continuation))))
  (let ((yang (display-*-and-return (get-current-continuation))))
    (yin yang)))

