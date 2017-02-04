;; Jump!
;; February 3, 2017
;; Jump is a simple one-player game:
;;
;; You are initially at the first cell of an array of cells containing
;; non-negative integers; at each step you can jump ahead in the array as
;; far as the integer at the current cell, or any smaller number of
;; cells. You win if there is a path that allows you to jump from one
;; cell to another, eventually jumping past the end of the array,
;; otherwise you lose. For instance, if the array contains the integers
;; [2, 0, 3, 5, 0, 0, 3, 0, 0, 3, 1, 0], you can win by jumping from 2,
;; to 3, to 5, to 3, to 3, then past the end of the array.
;;
;; Your task is to write a program that determines if a given game is
;; winnable. When you are finished, you are welcome to read or run a
;; suggested solution, or to post your own solution or discuss the
;; exercise in the comments below.

(defun winnablep (jumps)
  (labels ((winnable-position-p (position)
             (when (<= (length jumps) position)
               (return-from winnablep t))
             (let ((jumpable (aref jumps position)))
               (when (zerop jumpable)
                 (return-from winnable-position-p nil))
               (loop for jump downfrom jumpable to 1
                     if (winnable-position-p (+ position jump))
                       do (return-from winnablep t))
               nil)))
    (winnable-position-p 0)))

(winnablep #(2 0 3 5 0 0 3 0 0 3 1 0)) ; --> t
(winnablep #(2 0 3 1 0 0 3 0 0 3 1 0)) ; --> nil
(winnablep #(1)) ; --> t
(winnablep #(0)) ; --> nil
(winnablep #())  ; --> t


(defun winnable-path (jumps)
  (labels ((winnable-path-from-position (position)
             (if (<= (length jumps) position)
                 '(t)
                 (let ((jumpable (aref jumps position)))
                   (if (zerop jumpable)
                       nil
                       (loop for jump downfrom jumpable to 1
                             for path = (winnable-path-from-position (+ position jump))
                             if path
                               do (return (cons jump path))
                             finally (return nil)))))))
    (winnable-path-from-position 0)))

(winnable-path #(2 0 3 6 0 0 3 0 0 3 1 0)) ; --> (2 1 6 3 t)
(winnable-path #(2 0 3 5 0 0 3 0 0 3 1 0)) ; --> (2 1 3 3 3 t)
(winnable-path #(2 0 3 1 0 0 3 0 0 3 1 0)) ; --> nil
(winnable-path #(1)) ; --> (1 t)
(winnable-path #(0)) ; --> nil
(winnable-path #())  ; --> (t)
