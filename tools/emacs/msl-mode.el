;;; msl-mode.el --- Editing motor shading language -*- lexical-binding: t; -*-

;; SPDX-FileCopyrightText: 2023-2026 Alex Link
;; SPDX-License-Identifier: MIT

;;; Commentary:
;; A small C-derived mode. Public vocabulary follows msl/enums.hpp.
;; Highlighting is not validation of backend support or argument types.

;;; Code:

(require 'cc-mode)

(defconst msl-types
  '("void_t" "bool_t" "float_t" "int_t" "uint_t"
    "vec1_t" "vec2_t" "vec3_t" "vec4_t"
    "vec1i_t" "vec2i_t" "vec3i_t" "vec4i_t"
    "vec1ui_t" "vec2ui_t" "vec3ui_t" "vec4ui_t"
    "mat2_t" "mat3_t" "mat4_t"
    "tex1d_t" "tex2d_t" "tex2d_array_t" "data_buffer_t")
  "All public type spellings from the type mapping in enums.hpp.")

(defconst msl-builtins
  '("less_than" "less_than_equal" "greater_than" "greater_than_equal"
    "equal" "not_equal" "any" "all"
    "abs" "dot" "cross" "pulse" "smoothpulse" "step" "smoothstep"
    "clamp" "ceil" "floor" "mix" "pow" "min" "max" "fract" "mod"
    "texture" "rt_texcoords" "rt_texture" "rt_texture_offset"
    "as_vec2" "as_vec3" "as_vec4" "fetch_data" "texture_offset" "texture_dims"
    "ndc_coords" "linear_depth" "emit_vertex" "end_primitive"
    "rand_1d" "rand_4d" "noise_1d" "perlin_1d" "perlin_2d" "perlin_3d"
    "snoise_2d" "snoise_3d" "iqnoise" "fbm_1d")
  "All public function names in the msl buildins table.
Excludes the unknown sentinel and parser-only __make_array__.")

(defconst msl-bindings
  '("position" "normal" "tangent"
    "texcoord" "texcoord0" "texcoord1" "texcoord2" "texcoord3"
    "texcoord4" "texcoord5" "texcoord6" "texcoord7"
    "color" "color0" "color1" "color2" "color3" "color4" "color5" "color6" "color7"
    "depth" "projection" "view" "world" "object" "camera" "camera_position"
    "camera_clip" "light_direction" "light_position" "light_intensity" "light_color"
    "light_projection" "light_view" "light_shadow_map" "viewport"
    "vertex_id" "primitive_id" "instance_id")
  "All public binding names, including aliases, from `to_binding'.")

(defconst msl-keywords
  '("config" "library" "use" "shader" "render_states"
    "vertex_shader" "geometry_shader" "pixel_shader"
    "in" "out" "inout" "global" "local"
    "points" "lines" "triangles" "max_verts" "interleaved" "separate")
  "msl structural keywords; C control flow is inherited from `c-mode'.")

(defconst msl-native-functions
  '("sin" "cos" "tan" "asin" "acos" "atan" "atan2"
    "exp" "exp2" "log" "log2" "sqrt" "inversesqrt" "rsqrt"
    "sign" "length" "distance" "normalize" "reflect" "refract" "transpose")
  "Common target-language functions, not entries in the msl buildins table.
Highlighting these does not imply portability between all backends.")

(defun msl--code-matcher (regexp)
  "Make a font-lock matcher for REGEXP outside comments and strings."
  (lambda (limit)
    (let (found)
      (while (and (not found) (re-search-forward regexp limit t))
        (setq found
              (not (save-match-data
                     (save-excursion
                       (nth 8 (syntax-ppss (match-beginning 0))))))))
      found)))

(defconst msl-font-lock-keywords
  (mapcar
   (lambda (rule) (cons (msl--code-matcher (car rule)) (cdr rule)))
   `((,(regexp-opt msl-keywords 'symbols) . font-lock-keyword-face)
    (,(regexp-opt msl-types 'symbols) (0 font-lock-type-face t))
    (,(concat "\\(" (regexp-opt (append msl-builtins msl-native-functions) 'symbols)
              "\\)[ \t\n]*(")
     (1 font-lock-builtin-face t))
    (,(concat ":[ \t]*\\(" (regexp-opt msl-bindings 'symbols) "\\)")
     (1 font-lock-constant-face t))
    ("\\_<\\(?:config\\|library\\)\\_>[ \t]+\\([[:alpha:]_][[:alnum:]_.]*\\)"
     (1 font-lock-function-name-face t))))
  "Additional highlighting rules for msl.")

;;;###autoload
(define-derived-mode msl-mode c-mode "msl"
  "Major mode for motor shading language.
Uses C comments, strings, navigation and indentation.  This is not an msl
parser; C indentation may need manual adjustment for shader-specific blocks."
  (font-lock-add-keywords nil msl-font-lock-keywords 'append)
  (setq-local c-basic-offset 4))

;;;###autoload
(add-to-list 'auto-mode-alist '("\\.msl\\'" . msl-mode))

(provide 'msl-mode)
;;; msl-mode.el ends here
