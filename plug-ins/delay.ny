$nyquist plug-in
$version 4
$type process
$preview linear
$name (_ "Delay")
$manpage "Delay"
$debugbutton false
$action (_ "Applying Delay Effect...")
$author (_ "Steve Daulton")
$release 2.3.1
$copyright (_ "Released under terms of the GNU General Public License version 2")


;; Released under terms of the GNU General Public License version 2:
;; http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
;; based on 'Delay' by David R. Sky
;;
;; For information about writing and modifying Nyquist plug-ins:
;; https://wiki.audacityteam.org/wiki/Nyquist_Plug-ins_Reference


$control delay-type (_ "Delay type") choice ((_ "Regular")
                                             ("BouncingBall" (_ "Bouncing Ball"))
                                             ("ReverseBouncingBall" (_ "Reverse Bouncing Ball"))) 0
$control dgain (_ "Delay level per echo (dB)") real "" -6 -30 1
$control delay (_ "Delay time (seconds)") real "" 0.3 0 5
$control pitch-type (_ "Pitch change effect") choice (("PitchTempo" (_ "Pitch/Tempo"))
                                                      ("LQPitchShift" (_ "Low-quality Pitch Shift"))) 0 
$control shift (_ "Pitch change per echo (semitones)") real "" 0 -2 2
$control number (_ "Number of echoes") int "" 5 1 30
$control constrain (_ "Allow duration to change") choice ((_ "Yes")(_ "No")) 0 


;; The default pitch shift effect is a simple resampling, 
;; so both pitch and tempo of the delayed audio will change 
;; [as in Sneedacity's Change Speed effect].
;; LQ Pitch Shift (Low Quality) changes the pitch without 
;; changing the tempo, but the sound quality is not very good 
;; and tends to cause a short echo effect which can be quite 
;; noticeable on percussive sounds though may be acceptable 
;; on other sounds.


;;; Anti-alias low pass filter
(defun lp-wall (sig freq)
  (do ((count 0 (1+ count))
       (freq (* 0.94 freq)))
      ((= count 10) sig)
    (setf sig (lowpass8 sig freq))))

;;; Change speed
(defun change-speed (sig shift)
  (if (= shift 0)                               ; no pitch shift
    sig
    (let ((ratio (expt 0.5 (/ shift 12.0))))    ; shift value as frequency ratio
      (force-srate *sound-srate* 
        (stretch-abs ratio (sound sig))))))

;;; Pitch shift audio 
(defun p-shift (sig shift)
  (if (= shift 0)                               ; no pitch shift
    sig
    (let ((sig (force-srate 44100 sig))         ; pitshift quality best at 44100
          ; anti-alias filter frequency
          (minrate (* 0.5 (min *sound-srate* 44100)))
          (ratio (expt 0.5 (/ shift -12.0))))   ; shift value as frequency ratio
      (force-srate *sound-srate*                ; convert back to correct rate
        (progv '(*sound-srate*) (list 44100)    ; pitshift requires rates to match
          (cond 
            ((> shift 5)                        ; reduce aliasing
              (pitshift (lp-wall sig (/ minrate ratio)) ratio 1))
            ((< shift -2)
              (pitshift (hp sig 20) ratio 1))   ; reduce sub-sonic frequencies
            (T (pitshift sig ratio 1))))))))

;;; Apply effects to echo
(defun modify (sig num gain shift p-type)
  (let ((gain (db-to-linear (* num gain)))
        (shift (* num shift)))
    (if (= p-type 0)
        (mult gain (change-speed sig shift))
        (mult gain (p-shift sig shift)))))

;;; Compute echoes
(defun delays (sound gain delay shift num type mod)
  (let ((echo (s-rest 0)))
    (do ((count 1 (1+ count))
         (dly 0))
         ((> count num)(sim echo sound))
      (setq dly 
        (case type
          (0 (+ dly delay))
          (1 (+ dly (* delay (- (1+ num) count))))
          (2 (+ dly (* delay count)))))
      (setf echo (sim
        (at 0 (cue echo))
        (at-abs dly
          (cue (modify sound count gain shift mod))))))))

(defun constrain-abs (sig dur)
  (extract-abs 0 dur (cue sig)))


(let* ((delay (if (= delay-type 0)
                  delay
                  (/ delay number)))
       (output (multichan-expand #'delays *track*
                                          dgain
                                          delay
                                          shift
                                          number
                                          delay-type
                                          pitch-type)))
  (if (= constrain 1)
      (multichan-expand #'constrain-abs output (get-duration 1))
      output))
