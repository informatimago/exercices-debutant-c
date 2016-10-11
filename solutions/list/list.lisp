;; Comparer les fonctions C avec les fonctions lisp pour traiter des listes.
;; (Toute l'infrastructure de traitement des listes est déjà fournie par lisp).


(defun remove-odd-integers (list)
  (remove-if (function oddp) list))

(remove-odd-integers (loop for i below 20 collect i))
;; --> (0 2 4 6 8 10 12 14 16 18)


(defun remove-each-nth-element (list n)
  (remove-if (let ((i 0))
               (lambda (e)
                 (incf i)
                 (when (= i n)
                   (setf i 0))))
             list))

(remove-each-nth-element (loop for i below 20 collect i) 2)
;; --> (0 2 4 6 8 10 12 14 16 18)

(remove-each-nth-element (loop for i below 20 collect i) 3)
;; --> (0 1 3 4 6 7 9 10 12 13 15 16 18 19)


(defun swap-halves (list)
  (if (or (endp list)
          (endp (rest list)))
      list
      (labels ((split-points (turtle hare)
                 (cond
                   ((and hare (cdr hare) (cddr hare))
                    (split-points (cdr turtle) (cddr hare)))
                   ((cdr hare)
                    (values (cdr turtle) (cdr hare)))
                   (t
                    (values turtle hare)))))
        (multiple-value-bind (middle last) (split-points (cons nil list) list)
          (prog1 (rest middle)
            (setf (rest last) list
                  (rest middle) nil))))))

(loop
  for n below 10
  for l = (loop for i below n collect i)
  do (print l)
     (print (swap-halves l))
     (terpri))

;; nil 
;; nil 
;; 
;; (0) 
;; (0) 
;; 
;; (0 1) 
;; (1 0) 
;; 
;; (0 1 2) 
;; (1 2 0) 
;; 
;; (0 1 2 3) 
;; (2 3 0 1) 
;; 
;; (0 1 2 3 4) 
;; (2 3 4 0 1) 
;; 
;; (0 1 2 3 4 5) 
;; (3 4 5 0 1 2) 
;; 
;; (0 1 2 3 4 5 6) 
;; (3 4 5 6 0 1 2) 
;; 
;; (0 1 2 3 4 5 6 7) 
;; (4 5 6 7 0 1 2 3) 
;; 
;; (0 1 2 3 4 5 6 7 8) 
;; (4 5 6 7 8 0 1 2 3) 
;; nil



