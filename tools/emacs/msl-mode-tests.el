;;; msl-mode-tests.el --- Tests for msl-mode -*- lexical-binding: t; -*-

;; SPDX-FileCopyrightText: 2023-2026 Alex Link
;; SPDX-License-Identifier: MIT

(require 'ert)
(require 'msl-mode)

(defconst msl-test--enums-file
  (expand-file-name "../../msl/enums.hpp"
                    (file-name-directory (or load-file-name buffer-file-name))))

(defun msl-test--table (regexp &optional excluded)
  "Collect public name captures from REGEXP, ignoring EXCLUDED spellings."
  (with-temp-buffer
    (insert-file-contents msl-test--enums-file)
    (let (names)
      (while (re-search-forward regexp nil t)
        (let ((name (match-string-no-properties 1)))
          (unless (member name excluded) (push name names))))
      (sort (delete-dups names) #'string<))))

(ert-deftest msl-vocabulary-matches-source ()
  (should (equal (sort (copy-sequence msl-types) #'string<)
                 (msl-test--table
                  "__mapping_t[ \t]*( *\"\\([^\"]+\\)\" *, *type_t::")))
  (should (equal (sort (copy-sequence msl-builtins) #'string<)
                 (msl-test--table
                  "{ *buildin_type::[[:alnum:]_]+ *, *\"\\([^\"]+\\)\""
                  '("unknown-buildin" "__make_array__"))))
  (should (equal (sort (copy-sequence msl-bindings) #'string<)
                 (msl-test--table
                  "__mapping_t[ \t]*( *\"\\([^\"]+\\)\" *, *motor::msl::binding::"
                  '("unknown")))))

(defun msl-test--face (text word face)
  "Check FACE at WORD in a temporary msl buffer containing TEXT."
  (with-temp-buffer
    (insert text)
    (msl-mode)
    (font-lock-ensure)
    (goto-char (point-min))
    (search-forward word)
    (let ((actual (get-text-property (- (point) (length word)) 'face)))
      (should (if (listp actual) (memq face actual) (eq actual face))))))

(ert-deftest msl-types-and-builtins-highlight ()
  (dolist (name msl-types)
    (msl-test--face (concat name " value;\n") name 'font-lock-type-face))
  (dolist (name (append msl-builtins msl-native-functions))
    (msl-test--face (concat "void main() { " name "(p); }\n")
                    name 'font-lock-builtin-face)))

(ert-deftest msl-bindings-and-config-highlight ()
  (dolist (name msl-bindings)
    (msl-test--face (concat "vec4_t value : " name " ;\n") name 'font-lock-constant-face))
  (msl-test--face "config demo.light\n{\n}\n" "demo.light" 'font-lock-function-name-face)
  (msl-test--face "library demo.noise\n{\n}\n" "demo.noise" 'font-lock-function-name-face)
  (msl-test--face "vertex_shader\n{\n}\n" "vertex_shader" 'font-lock-keyword-face))

(ert-deftest msl-comments-and-strings-keep-their-faces ()
  (msl-test--face "// fbm_1d(p) vec3_t : position\n" "fbm_1d" 'font-lock-comment-face)
  (msl-test--face "/* vec3_t\n fbm_1d(p) */\n" "vec3_t" 'font-lock-comment-face)
  (msl-test--face "// config example\n" "example" 'font-lock-comment-face)
  (msl-test--face "\"fbm_1d(p) vec3_t\"\n" "fbm_1d" 'font-lock-string-face))

(ert-deftest msl-mode-selection-and-locality ()
  (with-temp-buffer
    (setq buffer-file-name "/tmp/example.lib.msl")
    (set-auto-mode)
    (should (eq major-mode 'msl-mode))
    (should (derived-mode-p 'c-mode))
    (should (= c-basic-offset 4)))
  (with-temp-buffer
    (c-mode)
    (should-not (member (car msl-font-lock-keywords) font-lock-keywords))))

;;; msl-mode-tests.el ends here
