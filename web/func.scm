(use-modules (ice-9 ftw))

(define (rgb r g b) (list r g b))
(define (rgb* v f) (map (λ (a) (* a f)) v))
(define (rgb+ a b) (if (or (nil? a) (nil? b)) '() (cons (+ (car a) (car b)) (rgb+ (cdr a) (cdr b)))))
(define (rgb->css v) (string-append "rgb("
				    (number->string (floor   (car v))) ","
				    (number->string (floor  (cadr v))) ","
				    (number->string (floor (caddr v))) ")"))

(define (rgb-interpolate a b f) (rgb+ (rgb* a (- 1.0 f)) (rgb* b f)))

(define (filesize path) (stat:size (stat path)))

(define (baseSF path) path)
(define (baseSFS path) path)
(define (hyper text)
  (let* ((tlen (string-length text)) (f 0.0) (i 0))
    (apply string-append (map (λ (v)
				(set! f (/ i tlen)) (set! i (+ 1 i))
				(string-append "<span style=\"color:" (rgb->css (rgb-interpolate (rgb 200 30 100) (rgb 100 200 30) f)) ";\">" (string v) "</span>")) (string->list text)))))


(define (get-newest-release-link arch branch)
  (let* ((prefix (string-append "wolkenwelten-" arch "-" branch)) (p-len (string-length prefix)))
    (string-append "releases/" arch "/" (car (last-pair (scandir (string-append "releases/" arch) (λ (path) (and (>= (string-length path) p-len) (string=? prefix (substring path 0 p-len))))))))))

(define (get-release-path-date path)
  (let* ((lst (string-split path #\-)) (len (length lst)))
    (format #nil "~a-~a-~a" (list-ref lst (- len 4)) (list-ref lst (- len 3)) (list-ref lst (- len 2)))))

(define (human-file-size bytes)
  (cond  [(> bytes (expt 2 20)) (format #nil "~0,1fMB" (/ bytes (expt 2 20)))]
         [(> bytes (expt 2 10)) (format #nil "~0,1fKB" (/ bytes (expt 2 10)))]
	 [#t (format #nil "~aB" bytes)]))

(define (get-release-title path)
  (format #nil "Released: ~a - Size: ~a" (get-release-path-date path) (human-file-size (filesize path))))

(define (get-newest-release-download name arch branch)
  (let ((path (get-newest-release-link arch branch)))
    (string-append "<a class=\"button\" href=\"" path "\" download title=\"" (get-release-title path) "\">" "<span class=\"buttonlabel\">" name "</span><span class=\"buttonicon icon-" arch "\"></span></a>")))

(define (stable-releases) "")
(define (experimental-releases)
  (string-append
   (get-newest-release-download "Windows"          "win"           "master")
   (get-newest-release-download "MacOS"            "macos"         "master")
   (get-newest-release-download "GNU/Linux"        "linux-x86_64"  "master")
   (get-newest-release-download "Linux ARM 64-Bit" "linux-aarch64" "master")
   (get-newest-release-download "Linux ARM 32-Bit" "linux-armv7l"  "master")))

(define (read-all path)
  (apply string (call-with-input-file path
		  (lambda (input-port)
		    (let loop ((x (read-char input-port)))
		      (cond
		       ((eof-object? x) '())
		       (#t (begin (cons x (loop (read-char input-port)))))))))))

(define (write-all path string)
  (call-with-output-file path
    (lambda (output-port)
      (display string output-port))))

(define (string-contains-all s1 s2 start)
  (let ((i (string-contains s1 s2 start)))
    (if i (cons i (string-contains-all s1 s2 (+ 1 i))) '())))

(define (zip a b)
  (if (or (null? a) (null? b)) '()
      (cons (cons (car a) (car b)) (zip (cdr a) (cdr b)))))

(define (valid-tag-list? l start)
  (if (null? l) #t (and (< start (caar l)) (< (caar l) (cdar l)) (valid-tag-list? (cdr l) (cdar l)))))

(define (build-string-list str lst start)
  (if (nil? lst)
      (cons (cons (substring str start (string-length str)) '()) '())
      (let* ((c-prefix (substring str start (caar lst)))
	     (c-code   (substring str       (+ 5 (caar lst)) (cdar lst))))
	(cons (cons c-prefix c-code) (build-string-list str (cdr lst) (+ 2 (cdar lst)))))))

(define (eval-list-tags lst)
  (map (λ (v) (if v (string-append (car v) (if (string? (cdr v)) (eval (call-with-input-string (cdr v) read) (interaction-environment)) "")) "")) lst))

(define (html-eval str)
  (let* ((open-i  (string-contains-all str "<?scm" 0))
         (close-i (string-contains-all str "?>" 0))
         (lst     (zip open-i close-i)))
    (unless (eq? (length open-i) (length close-i)) (display "Warning: Open and Close Tags do not seem to match!\n"))
    (if (valid-tag-list? lst 0) (apply string-append (eval-list-tags (build-string-list str lst 0))) str)))

(define (build)
  (write-all "index.html" (html-eval (read-all "template.html"))))

(build)
