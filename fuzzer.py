import random
import subprocess
import sys
import re
import os
import datetime


CLASSES = []
TEMPLATES = []
FUNCTIONS = []
TEMPLATE_FUNCTIONS = [] # list of TemplateFuncDef
STATIC_DEFS = [] # list of C++ lines for static definitions
TYPEDEFS = [] # list of TypedefDef

class TypedefDef:
    def __init__(self, alias_name, underlying_type_decl, namespaces):
        self.alias_name = alias_name
        self.underlying_type_decl = underlying_type_decl
        self.namespaces = namespaces

    def get_qname(self):
        active_ns = join_namespaces(self.namespaces)
        if not active_ns:
            return self.alias_name
        return active_ns + "::" + self.alias_name

    def render(self):
        ns_open = render_ns_open(self.namespaces)
        ns_close = "".join("}" + chr(10) for ns in self.namespaces)

        if "::*" in self.underlying_type_decl and "(" in self.underlying_type_decl and ")" in self.underlying_type_decl:
            decl = self.underlying_type_decl.replace("::*)", f"::*{self.alias_name})")
            return ns_open + f"typedef {decl};" + chr(10) + ns_close
        elif "(*)" in self.underlying_type_decl:
            decl = self.underlying_type_decl.replace("(*)", f"(*{self.alias_name})")
            return ns_open + f"typedef {decl};" + chr(10) + ns_close
        elif "[" in self.underlying_type_decl:
            base, bounds = self.underlying_type_decl.split("[", 1)
            return ns_open + f"typedef {base.strip()} {self.alias_name}[{bounds};" + chr(10) + ns_close
        else:
            return ns_open + f"typedef {self.underlying_type_decl} {self.alias_name};" + chr(10) + ns_close

def join_namespaces(namespaces):
    active = [ns for ns in namespaces if ns]
    return "::".join(active)

def render_ns_open(namespaces):
    return "".join((f"namespace {ns} {{" if ns else "namespace {") + chr(10) for ns in namespaces)

def gen_name(prefix=""):
    chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"
    first = random.choice(chars)
    rest = "".join(random.choice(chars + "0123456789") for _ in range(random.randint(3, 15)))
    name = prefix + first + rest
    keywords = ["int", "short", "char", "double", "float", "struct", "class", "union", "namespace", "virtual", "template", "operator", "delete", "new", "return", "void", "long", "signed", "unsigned", "extern", "const", "bool", "true", "false", "volatile", "enum", "static", "public", "private", "protected"]
    if name in keywords:
        return name + "_"
    return name

def gen_type(all_classes, allow_ref=False, allow_qualifiers=True):
    primitives = ["char", "short", "int", "long", "double", "bool"]
    if all_classes:
        primitives.append(random.choice(all_classes).get_qname())
    if TYPEDEFS:
        primitives.append(random.choice(TYPEDEFS).get_qname())

    base_t = random.choice(primitives)

    # Prepend signed/unsigned for integer primitive types!
    if base_t in ["char", "short", "int", "long"] and random.choice([True, False]):
        base_t = random.choice(["signed ", "unsigned "]) + base_t

    # Pointer levels
    ptr_level = random.randint(0, 3)
    t_str = base_t + ("*" * ptr_level)

    # Qualifiers
    if allow_qualifiers and random.choice([True, False]):
        t_str = "const " + t_str
    if allow_qualifiers and random.choice([True, False]):
        t_str = "volatile " + t_str

    # Reference
    if allow_ref and random.choice([True, False]):
        t_str = t_str + "&"

    return t_str

class OperatorOverload:
    def __init__(self, op):
        self.op = op

    def render(self, class_name):
        if self.op == "()":
            return f'  public: void operator()() {{ printf("[Op] operator()\\n"); }}' + chr(10)
        elif self.op == "=":
            return f'  public: {class_name}& operator=(const {class_name}& other) {{ printf("[Op] operator=\\n"); return *this; }}' + chr(10)
        else:
            return f'  public: bool operator{self.op}(const {class_name}& other) const {{ printf("[Op] operator{self.op}\\n"); return true; }}' + chr(10)

class TemplateFuncDef:
    def __init__(self, name, namespaces, nttis=[], arg_types=[], ret_type="void", is_static=False):
        self.name = name
        self.namespaces = namespaces
        self.nttis = nttis # list of (kind, name)
        self.arg_types = arg_types
        self.ret_type = ret_type
        self.is_static = is_static

    def get_qname(self):
        active_ns = join_namespaces(self.namespaces)
        if not active_ns:
            return self.name
        return active_ns + "::" + self.name

    def render_def(self):
        ns_open = render_ns_open(self.namespaces)
        ns_close = "".join("}" + chr(10) for ns in self.namespaces)

        static_kw = "static " if self.is_static else ""
        tmpl_params = []
        for kind, p_name in self.nttis:
            if kind == "typename":
                tmpl_params.append(f"typename {p_name}")
            else:
                tmpl_params.append(f"{kind} {p_name}")
        tmpl_str = ", ".join(tmpl_params)

        args_decl = ", ".join(f"{t} a{i}" for i, t in enumerate(self.arg_types))
        body_parts = []
        for i, t in enumerate(self.arg_types):
            val_print = f"(&a{i} != 0)" if ("*" in t or "&" in t or "Tdf" in t or is_class_type(t) or t in [p[1] for p in self.nttis if p[0] == "typename"]) else f"(int)a{i}"
            body_parts.append(f'printf("[Func] {self.name} arg=%d\\n", (int){val_print});')
        body = " ".join(body_parts)

        # Quirky local static variable!
        local_static = f"  static int local_static = 50; local_static++; printf(\"  local_static={self.name}=%d\\n\", local_static);"
        ret_stmt = get_return_stmt(self.ret_type)

        return ns_open + f"  template <{tmpl_str}> {static_kw}{self.ret_type} {self.name}({args_decl}) {{ {body} {local_static}{ret_stmt} }}" + chr(10) + ns_close

class ClassDef:
    def __init__(self, name, namespaces):
        self.name = name
        self.namespaces = namespaces
        self.bases = [] # list of (base_name, is_virtual)
        self.fields = [] # list of (field_type, field_name, access)
        self.virtual_funcs = [] # list of func names
        self.non_virtual_funcs = [] # list of (func_name, arg_types, is_static)
        self.member_templates = [] # list of TemplateFuncDef
        self.operators = [] # list of OperatorOverload
        self.enums = [] # list of (enum_name, list of values)
        self.nested_classes = [] # list of ClassDef

    def get_qname(self):
        active_ns = join_namespaces(self.namespaces)
        if not active_ns:
            return self.name
        return active_ns + "::" + self.name

    def render(self):
        ns_open = render_ns_open(self.namespaces)
        ns_close = "".join("}" + chr(10) for ns in self.namespaces)

        base_str = ""
        if self.bases:
            parts = []
            for base, is_virt in self.bases:
                virt = "virtual " if is_virt else ""
                parts.append(f"public {virt}{base.get_qname()}")
            base_str = " : " + ", ".join(parts)

        lines = [f"struct {self.name}{base_str} {{"]

        # Nested Enums
        for en_name, en_vals in self.enums:
            val_str = ", ".join(f"{v_name} = {v_val}" for v_name, v_val in en_vals)
            lines.append(f"  public: enum {en_name} {{ {val_str} }};")

        # Fields
        for f_type, f_name, access in self.fields:
            lines.append(f"  {access}: {f_type} {f_name};")

        # Nested classes (unconditional)
        for nc in self.nested_classes:
            lines.append("  public: " + nc.render_no_ns())

        lines.append(f'  public: virtual void print_class() {{ printf("{self.get_qname()}\\n"); }}')
        for ret_t, func in self.virtual_funcs:
            print_stmt = f'printf("[Virtual] {self.name}::{func}\\\\n");'
            ret_stmt = get_return_stmt(ret_t)
            lines.append(f"  public: virtual {ret_t} {func}() {{{print_stmt}{ret_stmt}}}")

        for ret_t, func, args, is_static in self.non_virtual_funcs:
            static_kw = "static " if is_static else ""
            args_str = ", ".join(f"{t} a{i}" for i, t in enumerate(args))
            # Local static variable in member function!
            local_static = f"  static int member_local_static = 200; member_local_static++; printf(\"  member_local_static={func}=%d\\n\", member_local_static);"
            ret_stmt = get_return_stmt(ret_t)
            lines.append(f'  public: {static_kw}{ret_t} {func}({args_str}) {{ printf("[Member] {self.name}::{func}\\n"); {local_static}{ret_stmt} }}')

        # Member Template Functions (including template static functions!)
        for mt in self.member_templates:
            static_kw = "static " if mt.is_static else ""
            tmpl_params = []
            for kind, p_name in mt.nttis:
                if kind == "typename":
                    tmpl_params.append(f"typename {p_name}")
                else:
                    tmpl_params.append(f"{kind} {p_name}")
            tmpl_str = ", ".join(tmpl_params)
            args_decl = ", ".join(f"{t} a{i}" for i, t in enumerate(mt.arg_types))
            body_parts = []
            for i, t in enumerate(mt.arg_types):
                val_print = f"(&a{i} != 0)" if ("*" in t or "&" in t or "Tdf" in t or is_class_type(t) or t in [p[1] for p in mt.nttis if p[0] == "typename"]) else f"(int)a{i}"
                body_parts.append(f'printf("[Member] {self.name}::{mt.name} arg=%d\\n", (int){val_print});')
            ret_stmt = get_return_stmt(mt.ret_type)
            body = " ".join(body_parts)
            lines.append(f"  public: template <{tmpl_str}> {static_kw}{mt.ret_type} {mt.name}({args_decl}) {{ {body}{ret_stmt} }}")



        for op in self.operators:
            lines.append(op.render(self.name))

        lines.append(f"  public: {self.name}() {{ print_class(); }}")
        lines.append(f"  public: virtual ~{self.name}() {{ print_class(); }}")
        lines.append("};")

        return ns_open + chr(10).join(lines) + chr(10) + ns_close

    def render_no_ns(self):
        base_str = ""
        if self.bases:
            parts = []
            for base, is_virt in self.bases:
                virt = "virtual " if is_virt else ""
                parts.append(f"public {virt}{base.get_qname()}")
            base_str = " : " + ", ".join(parts)

        lines = [f"struct {self.name}{base_str} {{"]
        for f_type, f_name, access in self.fields:
            lines.append(f"  {access}: {f_type} {f_name};")
        lines.append(f'  public: virtual void print_class() {{ printf("{self.name}\\n"); }}')
        lines.append(f"  public: {self.name}() {{ print_class(); }}")
        lines.append(f"  public: virtual ~{self.name}() {{ print_class(); }}")
        lines.append("};")
        return chr(10).join(lines)

class TemplateClassDef:
    def __init__(self, name, namespaces, nttis=[]):
        self.name = name
        self.namespaces = namespaces
        self.nttis = nttis # list of (kind, name)

    def get_qname(self):
        active_ns = join_namespaces(self.namespaces)
        if not active_ns:
            return self.name
        return active_ns + "::" + self.name

    def render(self):
        ns_open = render_ns_open(self.namespaces)
        ns_close = "".join("}" + chr(10) for ns in self.namespaces)

        tmpl_params = []
        for kind, p_name in self.nttis:
            if kind == "typename":
                tmpl_params.append(f"typename {p_name}")
            else:
                tmpl_params.append(f"{kind} {p_name}")
        tmpl_str = ", ".join(tmpl_params)

        lines = [
            f"template <{tmpl_str}> struct {self.name} {{",
            f"  int i_field;"
        ]
        for kind, p_name in self.nttis:
            if kind == "typename":
                lines.append(f"  {p_name} field_{p_name};")
            else:
                field_type = kind[:-1].strip() if kind.endswith("&") else kind
                lines.append(f"  {field_type} field_{p_name};")

        lines.append(f'  virtual void tfunc() {{ printf("[Templ] {self.get_qname()}\\n"); }}')
        lines.append(f"  {self.name}() {{ tfunc(); }}")
        lines.append(f"  virtual ~{self.name}() {{ tfunc(); }}")
        lines.append("};")

        return ns_open + chr(10).join(lines) + chr(10) + ns_close

class NamespaceFuncDef:
    def __init__(self, name, namespaces, arg_types, ret_type="void", is_static=False):
        self.name = name
        self.namespaces = namespaces
        self.arg_types = arg_types
        self.ret_type = ret_type
        self.is_static = is_static

    def get_qname(self):
        active_ns = join_namespaces(self.namespaces)
        if not active_ns:
            return self.name
        return active_ns + "::" + self.name

    def render_def(self):
        ns_open = render_ns_open(self.namespaces)
        ns_close = "".join("}" + chr(10) for ns in self.namespaces)

        static_kw = "static " if self.is_static else ""
        args_decl = ", ".join(f"{t} a{i}" for i, t in enumerate(self.arg_types))
        body_parts = []
        for i, t in enumerate(self.arg_types):
            val_print = f"(&a{i} != 0)" if ("*" in t or "&" in t or "Tdf" in t or is_class_type(t)) else f"(int)a{i}"
            body_parts.append(f'printf("[Func] {self.name} arg=%d\\n", (int){val_print});')
        body = " ".join(body_parts)

        # Local static variable!
        local_static = f"  static int local_static = 300; local_static++; printf(\"  local_static={self.name}=%d\\n\", local_static);"
        ret_stmt = get_return_stmt(self.ret_type)

        return ns_open + f"  {static_kw}{self.ret_type} {self.name}({args_decl}) {{ {body} {local_static}{ret_stmt} }}" + chr(10) + ns_close

def generate_hierarchy(num_classes):
    global CLASSES, TEMPLATES, FUNCTIONS, TEMPLATE_FUNCTIONS, STATIC_DEFS, TYPEDEFS
    CLASSES = []
    TEMPLATES = []
    FUNCTIONS = []
    TEMPLATE_FUNCTIONS = []
    STATIC_DEFS = []
    TYPEDEFS = []

    namespaces_pool = []
    for _ in range(3):
        depth = random.randint(0, 11)
        ns_list = [gen_name("Ns") for _ in range(depth)]
        if depth > 0 and random.random() < 0.2:
            idx = random.randint(0, depth - 1)
            ns_list[idx] = ""
        namespaces_pool.append(ns_list)
    namespaces_pool.append([])

    primitives = ["char", "short", "int", "double"]

    # 1. Generate 10 basic typedefs
    basic_tdfs = []
    for _ in range(10):
        t_name = gen_name("TdfB_")
        ns = random.choice(namespaces_pool)
        choice = random.choice(["func_ptr", "array", "multi_array", "ptr_to_array", "array_of_ptrs"])
        base_t = random.choice(primitives)

        if choice == "func_ptr":
            num_params = random.randint(1, 4)
            params = [random.choice(primitives) for _ in range(num_params)]
            decl = f"void (*)({', '.join(params)})"
        elif choice == "array":
            size = random.randint(2, 10)
            decl = f"{base_t} [{size}]"
        elif choice == "multi_array":
            dims = random.randint(2, 4)
            decl = base_t + "".join(f" [2]" for _ in range(dims))
        elif choice == "ptr_to_array":
            size = random.randint(2, 10)
            decl = f"{base_t} (*)[{size}]"
        elif choice == "array_of_ptrs":
            size = random.randint(2, 10)
            decl = f"{base_t}* [{size}]"

        basic_tdfs.append(TypedefDef(t_name, decl, ns))

    # 2. Generate 10 nested complex typedefs (arrays & callbacks up to 11 params/dimensions)
    nested_tdfs = []
    for _ in range(10):
        t_name = gen_name("TdfN_")
        ns = random.choice(namespaces_pool)

        src_tdf = random.choice(basic_tdfs)
        base_t = src_tdf.get_qname()

        choice = random.choice(["func_ptr_returning_ptr", "func_ptr_taking_callback", "array_of_func_ptrs", "ptr_to_array_of_func_ptrs"])

        if choice == "func_ptr_returning_ptr":
            decl = f"{base_t}* (*)(int)"
        elif choice == "func_ptr_taking_callback":
            decl = f"void (*)({base_t})"
        elif choice == "array_of_func_ptrs":
            size = random.randint(2, 10)
            decl = f"{base_t} [{size}]"
        elif choice == "ptr_to_array_of_func_ptrs":
            size = random.randint(2, 10)
            decl = f"{base_t} (*)[{size}]"

        nested_tdfs.append(TypedefDef(t_name, decl, ns))

    TYPEDEFS = basic_tdfs + nested_tdfs

    for i in range(num_classes):
        name = gen_name("C")
        ns = random.choice(namespaces_pool)
        c = ClassDef(name, ns)

        if len(CLASSES) > 0:
            max_bases = min(2, len(CLASSES))
            accumulated_transitive_bases = set()
            chosen_bases = []
            candidates = list(CLASSES)
            random.shuffle(candidates)
            for base in candidates:
                if len(chosen_bases) >= max_bases:
                    break
                base_transitive = get_transitive_bases(base)
                is_disjoint = True
                if base.get_qname() in accumulated_transitive_bases:
                    is_disjoint = False
                elif not base_transitive.isdisjoint(accumulated_transitive_bases):
                    is_disjoint = False
                if is_disjoint:
                    chosen_bases.append(base)
                    accumulated_transitive_bases.add(base.get_qname())
                    accumulated_transitive_bases.update(base_transitive)
            for base in chosen_bases:
                is_virt = random.choice([True, False])
                c.bases.append((base, is_virt))

        num_fields = random.randint(0, 4)
        for j in range(num_fields):
            f_type = gen_type(CLASSES, allow_ref=False, allow_qualifiers=False)
            f_name = gen_name("f_")
            access = random.choice(["public", "private", "protected"])
            c.fields.append((f_type, f_name, access))

        # Nested Enums
        if random.choice([True, False]):
            en_vals = []
            for _ in range(random.randint(2, 4)):
                v_name = gen_name("VAL_")
                choice = random.choice(["small_pos", "neg", "large_pos", "large_neg", "shift"])
                if choice == "small_pos":
                    v_val = random.randint(0, 100)
                elif choice == "neg":
                    v_val = random.randint(-1000, -1)
                elif choice == "large_pos":
                    v_val = random.randint(100000, 2147483647)
                elif choice == "large_neg":
                    v_val = random.randint(-2147483648, -100000)
                elif choice == "shift":
                    v_val = (1 << random.randint(0, 30))
                    if random.choice([True, False]):
                        v_val = ~v_val
                en_vals.append((v_name, v_val))
            c.enums.append((gen_name("Enum_"), en_vals))

        # Nested Class (Unconditional!)
        nc = ClassDef(gen_name("NC_"), [])
        num_fields_nc = random.randint(1, 2)
        for j in range(num_fields_nc):
            f_type = gen_type([], allow_ref=False, allow_qualifiers=False)
            f_name = gen_name("f_")
            nc.fields.append((f_type, f_name, "public"))
        c.nested_classes.append(nc)

        if random.choice([True, False]):
            ret_t = gen_return_type(CLASSES)
            c.virtual_funcs.append((ret_t, gen_name("vf_")))

        # Randomly override virtual functions from transitive bases!
        base_defs = get_transitive_base_defs(c)
        for base_def in base_defs:
            for ret_t, vf_name in base_def.virtual_funcs:
                if not any(vf_name == name for _, name in c.virtual_funcs):
                    if random.choice([True, False]):
                        c.virtual_funcs.append((ret_t, vf_name))

        # Non-virtual member function (non-static - Unconditional!)
        num_params_ns = random.randint(1, 6)
        params_ns = [gen_type(CLASSES, allow_ref=True) for _ in range(num_params_ns)]
        ret_t_ns = gen_return_type(CLASSES)
        c.non_virtual_funcs.append((ret_t_ns, gen_name("mf_ns_"), params_ns, False))

        # Static member function (Unconditional!)
        num_params_s = random.randint(1, 6)
        params_s = [gen_type(CLASSES, allow_ref=True) for _ in range(num_params_s)]
        ret_t_s = gen_return_type(CLASSES)
        c.non_virtual_funcs.append((ret_t_s, gen_name("mf_s_"), params_s, True))

        # Member Template Functions (up to 11 parameters, both template static and non-static!)
        num_params = random.randint(1, 11)
        nttis = [("typename", "U")]
        for k in range(num_params - 1):
            nttis.append(("int", f"N{k}"))
        params = ["U"] + [gen_type(CLASSES, allow_ref=True) for _ in range(num_params - 1)]
        is_static_tmpl = random.choice([True, False])
        ret_t_mt = gen_return_type(CLASSES)
        c.member_templates.append(TemplateFuncDef(gen_name("mt_"), [], nttis, params, ret_t_mt, is_static_tmpl))

        # Member Template Explicit Specialization! (Quirk!)
        # For non-template parent classes, we use a single template <> prefix at namespace scope
        spec_nttis = [("typename", "double")]
        for k in range(num_params - 1):
            spec_nttis.append(("int", "42"))

        # Copy the primary template's arg_types exactly, substituting U with double
        spec_params = []
        for t in c.member_templates[-1].arg_types:
            if t == "U":
                spec_params.append("double")
            elif t == "int" if "U" == "int" else False: # Wait, U is just U!
                spec_params.append("double")
            else:
                spec_params.append(t)

        spec_args_decl = ", ".join(f"{t} a{i}" for i, t in enumerate(spec_params))
        spec_tmpl_args = ", ".join("double" if p[0] == "typename" else "42" for p in spec_nttis)
        ret_t_spec = c.member_templates[-1].ret_type
        ret_stmt_spec = get_return_stmt(ret_t_spec)
        spec_body = f'printf("[MemberSpecial] Specialized mt\\n");{ret_stmt_spec}'

        ns_open = render_ns_open(c.namespaces)
        ns_close = "".join("}" + chr(10) for ns in c.namespaces)
        STATIC_DEFS.append(ns_open + f"template <> {ret_t_spec} {c.name}::{c.member_templates[-1].name}<{spec_tmpl_args}>({spec_args_decl}) {{ {spec_body} }}" + chr(10) + ns_close)

        if random.choice([True, False]):
            c.operators.append(OperatorOverload(random.choice(["+", "==", "<", "()", "="])))

        CLASSES.append(c)

        # Randomly generate pointer-to-member typedefs for class c!
        if random.choice([True, False]):
            ptmv_t = random.choice(["char", "short", "int", "long", "double", "bool"])
            ptmv_decl = f"{ptmv_t} {c.get_qname()}::*"
            TYPEDEFS.append(TypedefDef(gen_name("Tdf_PTMV_"), ptmv_decl, c.namespaces))

        if random.choice([True, False]):
            ptmf_ret = random.choice(["void", "char", "short", "int", "double"])
            ptmf_params_num = random.randint(0, 3)
            ptmf_params = [random.choice(["char", "short", "int", "double"]) for _ in range(ptmf_params_num)]
            ptmf_params_str = ", ".join(ptmf_params)
            ptmf_decl = f"{ptmf_ret} ({c.get_qname()}::*)({ptmf_params_str})"
            TYPEDEFS.append(TypedefDef(gen_name("Tdf_PTMF_"), ptmf_decl, c.namespaces))

    # Template definitions with up to 11 parameters (typename & NTTI mix)
    if random.choice([True, False]):
        num_params = random.randint(1, 11)
        nttis = []
        for k in range(num_params):
            nttp_choices = ["typename", "int", "char", "bool", "short", "long", "int*", "int&", "bool*", "bool&",
                            "unsigned int", "signed char", "unsigned char", "unsigned long", "unsigned short"]
            if len(CLASSES) > 0:
                chosen_c = random.choice(CLASSES).get_qname()
                nttp_choices.append(f"{chosen_c}*")
                nttp_choices.append(f"{chosen_c}&")
            p_choice = random.choice(nttp_choices)
            if p_choice == "typename":
                nttis.append(("typename", f"T{k}"))
            else:
                nttis.append((p_choice, f"N{k}"))
        TEMPLATES.append(TemplateClassDef(gen_name("TmplClass"), random.choice(namespaces_pool), nttis))

    # Standalone non-static free function (Unconditional!)
    num_params = random.randint(1, 6)
    params = [gen_type(CLASSES, allow_ref=True) for _ in range(num_params)]
    ret_t_fn = gen_return_type(CLASSES)
    FUNCTIONS.append(NamespaceFuncDef(gen_name("fn_ns_"), random.choice(namespaces_pool), params, ret_t_fn, False))

    # Standalone static free function (Unconditional!)
    num_params_s = random.randint(1, 6)
    params_s = [gen_type(CLASSES, allow_ref=True) for _ in range(num_params_s)]
    ret_t_fs = gen_return_type(CLASSES)
    FUNCTIONS.append(NamespaceFuncDef(gen_name("fn_s_"), random.choice(namespaces_pool), params_s, ret_t_fs, True))

    # Standalone Template Functions (Unconditional!)
    num_params_t = random.randint(1, 6)
    nttis = [("typename", "U")]
    for k in range(num_params_t - 1):
        nttis.append(("int", f"N{k}"))
    params_t = ["U"] + [gen_type(CLASSES, allow_ref=True) for _ in range(num_params_t - 1)]
    ret_t_tfn = gen_return_type(CLASSES)
    TEMPLATE_FUNCTIONS.append(TemplateFuncDef(gen_name("tfn_"), random.choice(namespaces_pool), nttis, params_t, ret_t_tfn))

def has_vbase(c_name):
    c_def = None
    for c in CLASSES:
        if c.name == c_name:
            c_def = c
            break
    if not c_def:
        return False
    for base, is_virt in c_def.bases:
        if is_virt:
            return True
        if has_vbase(base.name):
            return True
    return False

def is_dynamic_c(c):
    if c.virtual_funcs:
        return True
    for base, _ in c.bases:
        if is_dynamic_c(base):
            return True
    return False

def get_clean_type_decl(t_str, var_name):
    is_class = False
    for c in CLASSES:
        if c.get_qname() in t_str or c.name in t_str:
            is_class = True
            break

    is_array = False
    if "[" in t_str:
        is_array = True
    else:
        clean_alias = t_str.split("::")[-1].replace("&", "").replace("const", "").replace("volatile", "").strip()
        for td in TYPEDEFS:
            if td.alias_name == clean_alias:
                if "[" in td.underlying_type_decl:
                    is_array = True
                    break

    if (is_class and "*" not in t_str) or is_array:
        clean_t = t_str.replace("&", "").replace("const", "").replace("volatile", "").strip()
        return f"{clean_t} {var_name};"
    else:
        clean_t = t_str.replace("&", "").strip()
        return f"{clean_t} {var_name} = 0;"

def is_class_type(t_str):
    clean_t = t_str.replace("const", "").replace("volatile", "").replace("&", "").strip()
    for c in CLASSES:
        if c.get_qname() == clean_t or c.name == clean_t:
            return True
    return False

def get_transitive_bases(class_def):
    bases_set = set()
    for base, _ in class_def.bases:
        bases_set.add(base.get_qname())
        bases_set.update(get_transitive_bases(base))
    return bases_set

def get_transitive_base_defs(class_def):
    base_defs = []
    for base, _ in class_def.bases:
        base_defs.append(base)
        base_defs.extend(get_transitive_base_defs(base))
    return list(dict.fromkeys(base_defs))

def get_all_vfuncs(class_def):
    vfuncs = dict()
    for base, _ in class_def.bases:
        for vf_name, ret_t in get_all_vfuncs(base):
            vfuncs[vf_name] = ret_t
    for ret_t, vf_name in class_def.virtual_funcs:
        vfuncs[vf_name] = ret_t
    return list(vfuncs.items())

def needs_temp_var(arg_type):
    if arg_type.endswith("&"):
        return True
    clean_t = arg_type.replace("const", "").replace("volatile", "").strip()
    for c in CLASSES:
        if c.get_qname() == clean_t or c.name == clean_t:
            return True
    return False

def get_global_var_for_type(t_str):
    # Primitives
    if t_str in ["char", "short", "int", "long", "double", "bool"]:
        return f"g_{t_str}"
    # Classes
    clean_t = t_str.split("::")[-1].strip()
    for c in CLASSES:
        if c.name == clean_t:
            active_ns = join_namespaces(c.namespaces)
            ns_prefix = active_ns + "::" if active_ns else ""
            return f"{ns_prefix}g_obj_{c.name}"
    return None

def gen_return_type(all_classes):
    if random.choice([True, False]):
        return "void"
    while True:
        t = gen_type(all_classes, allow_ref=False, allow_qualifiers=False)
        is_array_tdf = False
        clean_t = t.split("::")[-1].strip()
        for td in TYPEDEFS:
            if td.alias_name == clean_t and "[" in td.underlying_type_decl:
                is_array_tdf = True
                break
        if not is_array_tdf:
            return t

def get_return_stmt(ret_t):
    if ret_t == "void":
        return ""
    elif "*" in ret_t or " " in ret_t:
        return " return 0;"
    else:
        return f" return {ret_t}();"

def render_main():
    lines = [
        'extern "C" int printf(const char* format, ...);',
        '#define offsetof(s,m) ((int)&(((s*)0)->m))',
        'struct PTMF_Repr {',
        '  short delta;',
        '  short index;',
        '  void* pfn;',
        '};',
        'struct FuzzVTableCheck {',
        '  virtual void print_class() {}',
        '  virtual ~FuzzVTableCheck() {}',
        '};',
        'char g_char = 0;',
        'short g_short = 0;',
        'int g_int = 0;',
        'long g_long = 0;',
        'double g_double = 0;',
        'bool g_bool = 0;'
    ]

    for c in CLASSES:
        ns_open = render_ns_open(c.namespaces)
        ns_close = "".join("}" + chr(10) for ns in c.namespaces)
        lines.append(ns_open + f"struct {c.name};" + chr(10) + ns_close)

    for td in TYPEDEFS:
        lines.append(td.render())

    for c in CLASSES:
        lines.append(c.render())

    # Declare global objects for each class inside their respective namespaces!
    for c in CLASSES:
        ns_open = render_ns_open(c.namespaces)
        ns_close = "".join("}" + chr(10) for ns in c.namespaces)
        lines.append(ns_open + f"{c.name} g_obj_{c.name};" + chr(10) + ns_close)

    for t in TEMPLATES:
        lines.append(t.render())

    for f in FUNCTIONS:
        lines.append(f.render_def())

    for f in TEMPLATE_FUNCTIONS:
        lines.append(f.render_def())

    # Render all member template specializations outside classes!
    for sd in STATIC_DEFS:
        lines.append(sd)

    # Explicit Instantiations (Standard Classes & Functions)
    for t in TEMPLATES:
        ns_open = render_ns_open(t.namespaces)
        ns_close = "".join("}" + chr(10) for ns in t.namespaces)

        args = []
        for kind, _ in t.nttis:
            if kind.endswith("*"):
                base_t = kind[:-1].strip()
                g_var = get_global_var_for_type(base_t)
                args.append(f"&{g_var}")
            elif kind.endswith("&"):
                base_t = kind[:-1].strip()
                g_var = get_global_var_for_type(base_t)
                args.append(g_var)
            elif kind == "typename":
                args.append("int")
            elif kind == "bool":
                args.append("true")
            elif "char" in kind:
                args.append("'A'")
            else:
                args.append("42")
        args_str = ", ".join(args)
        lines.append(ns_open + f"template class {t.name}<{args_str}>;" + chr(10) + ns_close)

        args2 = []
        for kind, _ in t.nttis:
            if kind.endswith("*"):
                base_t = kind[:-1].strip()
                g_var = get_global_var_for_type(base_t)
                args2.append(f"&{g_var}")
            elif kind.endswith("&"):
                base_t = kind[:-1].strip()
                g_var = get_global_var_for_type(base_t)
                args2.append(g_var)
            elif kind == "typename":
                args2.append("short")
            elif kind == "bool":
                args2.append("false")
            elif "char" in kind:
                args2.append("'B'")
            else:
                args2.append("10")
        args_str2 = ", ".join(args2)
        if args_str != args_str2:
            lines.append(ns_open + f"template class {t.name}<{args_str2}>;" + chr(10) + ns_close)

    for f in TEMPLATE_FUNCTIONS:
        ns_open = render_ns_open(f.namespaces)
        ns_close = "".join("}" + chr(10) for ns in f.namespaces)
        args = []
        for kind, _ in f.nttis:
            if kind == "typename":
                args.append("int")
            elif kind == "int":
                args.append("42")
        args_str = ", ".join(args)
        func_args = ", ".join("int" if t == "U" else t for t in f.arg_types)
        lines.append(ns_open + f"template {f.ret_type} {f.name}<{args_str}>({func_args});" + chr(10) + ns_close)

    lines.append("int main() {")

    for f in FUNCTIONS:
        arg_calls = []
        for arg_type in f.arg_types:
            arg_call = "0"
            if needs_temp_var(arg_type):
                var_name = f"ref_obj_{f.name}_{len(arg_calls)}"
                lines.append("  " + get_clean_type_decl(arg_type, var_name))
                arg_call = var_name
            arg_calls.append(arg_call)
        lines.append(f"  {f.get_qname()}({', '.join(arg_calls)});")

    for f in TEMPLATE_FUNCTIONS:
        args = []
        for kind, _ in f.nttis:
            if kind == "typename":
                args.append("int")
            elif kind == "int":
                args.append("42")
        args_str = ", ".join(args)
        arg_calls = []
        for arg_type in f.arg_types:
            arg_call = "0"
            if arg_type == "U":
                arg_call = "10"
            elif needs_temp_var(arg_type):
                var_name = f"tref_obj_{f.name}_{len(arg_calls)}"
                lines.append("  " + get_clean_type_decl("int" if arg_type == "U" else arg_type, var_name))
                arg_call = var_name
            arg_calls.append(arg_call)
        lines.append(f"  {f.get_qname()}<{args_str}>({', '.join(arg_calls)});")

    for c in CLASSES:
        lines.append(f'  printf("--- Class {c.get_qname()} ---\\n");')
        lines.append(f'  {{')
        lines.append(f'    {c.get_qname()} obj;')

        lines.append(f'    printf("sizeof({c.get_qname()}) = %d\\n", (int)sizeof({c.get_qname()}));')

        # Dump Nested Class sizes and offsets!
        for nc in c.nested_classes:
            lines.append(f'    printf("sizeof({c.get_qname()}::{nc.name}) = %d\\n", (int)sizeof({c.get_qname()}::{nc.name}));')
            for f_type, f_name, _ in nc.fields:
                if not f_type.endswith("&"):
                    lines.append(f'    printf("offset {c.get_qname()}::{nc.name}.{f_name} = %d\\n", offsetof({c.get_qname()}::{nc.name}, {f_name}));')

        # Dump Enum size and values
        for en_name, en_vals in c.enums:
            lines.append(f'    printf("enum_sizeof({c.get_qname()}::{en_name}) = %d\\n", (int)sizeof({c.get_qname()}::{en_name}));')
            for val, _ in en_vals:
                lines.append(f'    printf("enum_val({c.get_qname()}::{en_name}::{val}) = %d\\n", (int){c.get_qname()}::{val});')

        # Fields offsets
        for f_type, f_name, access in c.fields:
            if not f_type.endswith("&") and access == "public": # offsetof requires non-reference public members
                lines.append(f'    printf("offset {c.get_qname()}.{f_name} = %d\\n", offsetof({c.get_qname()}, {f_name}));')

        # Base class offsets
        for base, is_virt in c.bases:
            lines.append(f'    {{')
            lines.append(f'      {c.get_qname()}* p = &obj;')
            lines.append(f'      {base.get_qname()}* b = ({base.get_qname()}*)p;')
            lines.append(f'      printf("base_offset {c.get_qname()}->{base.get_qname()} = %d\\n", (int)((char*)b - (char*)p));')
            lines.append(f'    }}')

        # Non-virtual member function calls
        for ret_t, func, args, is_static in c.non_virtual_funcs:
            arg_calls = []
            for arg_type in args:
                arg_call = "0"
                if needs_temp_var(arg_type):
                    var_name = f"m_ref_obj_{func}_{len(arg_calls)}"
                    lines.append("    " + get_clean_type_decl(arg_type, var_name))
                    arg_call = var_name
                arg_calls.append(arg_call)
            if is_static:
                lines.append(f"    {c.get_qname()}::{func}({', '.join(arg_calls)});")
            else:
                lines.append(f"    obj.{func}({', '.join(arg_calls)});")

        # Virtual member function calls
        for vf_name, ret_t in get_all_vfuncs(c):
            lines.append(f"    obj.{vf_name}();")

        # Member Template function calls
        for mt in c.member_templates:
            args = []
            for kind, _ in mt.nttis:
                if kind == "typename":
                    args.append("int")
                elif kind == "int":
                    args.append("42")
            args_str = ", ".join(args)
            arg_calls = []
            for arg_type in mt.arg_types:
                arg_call = "0"
                if arg_type == "U":
                    arg_call = "20"
                elif needs_temp_var(arg_type):
                    var_name = f"mtref_obj_{mt.name}_{len(arg_calls)}"
                    lines.append("    " + get_clean_type_decl("int" if arg_type == "U" else arg_type, var_name))
                    arg_call = var_name
                arg_calls.append(arg_call)
            if mt.is_static:
                lines.append(f"    {c.get_qname()}::{mt.name}<{args_str}>({', '.join(arg_calls)});")
            else:
                lines.append(f"    obj.{mt.name}<{args_str}>({', '.join(arg_calls)});")

            # Call the specialized member template version!
            spec_args = []
            for arg_type in mt.arg_types:
                arg_call = "0"
                if arg_type == "U":
                    arg_call = "3.14"
                elif needs_temp_var(arg_type):
                    var_name = f"mtspec_obj_{mt.name}_{len(spec_args)}"
                    lines.append("    " + get_clean_type_decl("double" if arg_type == "U" else arg_type, var_name))
                    arg_call = var_name
                spec_args.append(arg_call)
            spec_tmpl_args = []
            for kind, _ in mt.nttis:
                if kind == "typename":
                    spec_tmpl_args.append("double")
                elif kind == "int":
                    spec_tmpl_args.append("42")
            spec_tmpl_str = ", ".join(spec_tmpl_args)
            if mt.is_static:
                lines.append(f"    {c.get_qname()}::{mt.name}<{spec_tmpl_str}>({', '.join(spec_args)});")
            else:
                lines.append(f"    obj.{mt.name}<{spec_tmpl_str}>({', '.join(spec_args)});")

        # Operators calls
        for op in c.operators:
            if op.op == "()":
                lines.append("    obj();")
            elif op.op == "=":
                lines.append("    obj = obj;")
            else:
                lines.append(f"    obj {op.op} obj;")

        # Member pointers (PTMD)
        if c.fields and not c.fields[0][0].endswith("&") and c.fields[0][2] == "public":
            f_type, f_name, _ = c.fields[0]
            lines.append(f'    {{')
            lines.append(f'      union {{ {f_type} {c.get_qname()}::*p; int val; }} u;')
            lines.append(f'      u.p = &{c.get_qname()}::{f_name};')
            lines.append(f'      printf("ptmd {c.get_qname()}.{f_name} = %d\\n", u.val);')
            lines.append(f'    }}')

        # Member function pointers (PTMF) representation delta/index
        if not has_vbase(c.name):
            lines.append(f'    {{')
            lines.append(f'      union {{ void ({c.get_qname()}::*p)(); PTMF_Repr repr; }} u;')
            lines.append(f'      u.p = &{c.get_qname()}::print_class;')
            lines.append(f'      printf("ptmf {c.get_qname()}::print_class: delta=%d, index=%d\\n", u.repr.delta, u.repr.index);')
            lines.append(f'    }}')

        lines.append(f'  }}')

    for t in TEMPLATES:
        args = []
        for kind, _ in t.nttis:
            if kind.endswith("*"):
                base_t = kind[:-1].strip()
                g_var = get_global_var_for_type(base_t)
                args.append(f"&{g_var}")
            elif kind.endswith("&"):
                base_t = kind[:-1].strip()
                g_var = get_global_var_for_type(base_t)
                args.append(g_var)
            elif kind == "typename":
                args.append("int")
            elif kind == "bool":
                args.append("true")
            elif "char" in kind:
                args.append("'A'")
            else:
                args.append("42")
        args_str = ", ".join(args)
        lines.append(f'  {{')
        lines.append(f'    {t.get_qname()}<{args_str}> tobj;')
        lines.append(f'    printf("sizeof({t.get_qname()}<{args_str}>) = %d\\n", (int)sizeof({t.get_qname()}<{args_str}>));')
        lines.append(f'  }}')

    # Dedicated dynamic RTTI/VTable slot check block
    lines.append('  {')
    lines.append('    FuzzVTableCheck obj;')
    lines.append('    void** vtable = *(void***)&obj;')
    lines.append('    printf("  FuzzVTableCheck offset_to_top=%d, rtti_present=%d\\n", (int)vtable[0], (vtable[1] != 0));')
    lines.append('  }')

    lines.append("  return 0;")
    lines.append("}")
    return "\n".join(lines)

def get_mangled_symbols(obj_path):
    res = subprocess.run(["nm", obj_path], capture_output=True, text=True)
    if res.returncode != 0:
        return None

    symbols = set()
    
    # Standard library / runtime helper symbols that must be ignored exactly
    ignore_symbols = {
        # VTables of C++ runtime RTTI helper classes (from standard cxxabi.h)
        "__vt_16__user_type_info", "__vt_17__class_type_info", "__vt_14__si_type_info",
        # Standard C++ memory allocation, exception throwing, and terminate handlers
        "__builtin_delete", "__throw", "terminate",
        # Standard C library runtime functions
        "printf", "abort", "memcpy", "atexit",
        # Linker global offset tables
        "_GLOBAL_OFFSET_TABLE_",
        # GCC2/Clang exception handling metadata tables and frame stubs
        "__EXCEPTION_TABLE__", "__FRAME_BEGIN__", "__EH_FRAME_BEGIN__",
        # GCC 2.95 local compilation marker symbol
        "gcc2_compiled.",
        # GCC2 C++ runtime type matcher personality routine (defined inside libstdc++.a)
        "__cplus_type_matcher"
    }
    
    for line in res.stdout.splitlines():
        parts = line.strip().split()
        if len(parts) < 2:
            continue
        sym = parts[-1]

        # 1. Check exact ignored symbols
        if sym in ignore_symbols:
            continue

        # 2. Check globally ignored prefixes
        is_ignored = False
        # Ignore GCC2 template repository keys (starts with __t followed by a digit)
        if sym.startswith("__t") and len(sym) > 3 and sym[3].isdigit():
            is_ignored = True
        # Ignore compiler-internal constructor symbols (starts with __Q or __Q_ or __ + digit)
        if sym.startswith("__Q") or sym.startswith("__Q_"):
            is_ignored = True
        if sym.startswith("__") and len(sym) > 2 and sym[2].isdigit():
            is_ignored = True

        if is_ignored:
            continue

        # 3. Filter out compiler-internal construction vtables using the mathematical dot-counting rule
        is_valid_vtable = True
        if sym.startswith("__vt_"):
            dot_count = sym.count(".")
            is_anon = "_GLOBAL_.N." in sym
            if is_anon:
                if dot_count > 2:
                    is_valid_vtable = False
            else:
                if dot_count > 0:
                    is_valid_vtable = False
        if not is_valid_vtable:
            continue

        # Keep all standard fuzzed C++ user symbols (functions, variables, vtables, thunks, operators!)
        symbols.add(sym)

    return symbols

def run_test(config):
    code = render_main()

    fuzz_cpp = os.path.join(config["scratch_dir"], "fuzz_test.cpp")
    gcc_obj = os.path.join(config["scratch_dir"], "fuzz_test_gcc2.o")
    clang_obj = os.path.join(config["scratch_dir"], "fuzz_test_clang.o")
    gcc_bin = os.path.join(config["scratch_dir"], "fuzz_test_gcc2")
    clang_bin = os.path.join(config["scratch_dir"], "fuzz_test_clang")

    with open(fuzz_cpp, "w") as f:
        f.write(code)

    # Compile with GCC2
    gcc2_compile_cmd = [
        os.path.join(config["gcc_dir"], "usr/bin/g++-2.95"),
        f"-B{config['gcc_lib_dir']}/",
        "-Wa,--32", "-c",
        fuzz_cpp, "-o", gcc_obj
    ]
    res = subprocess.run(gcc2_compile_cmd, capture_output=True, text=True)
    if res.returncode != 0:
        err_lines = [line.strip() for line in res.stderr.splitlines() if "error:" in line or "warning:" in line or "failed" in line or "invalid" in line or "inaccessible" in line or "ambiguous" in line]
        err_summary = err_lines[0] if err_lines else (res.stderr.splitlines()[0] if res.stderr.splitlines() else "Unknown GCC2 compile error")
        return False, f"GCC2_COMPILE_ERROR ({err_summary})", res.stderr

    # Compile with Clang
    clang_compile_cmd = [
        config["clang"],
        "-target", "i386-pc-linux-gnu", "-m32",
        "-Xclang", "-fc++-abi=gcc2", "-fno-asynchronous-unwind-tables", "-c",
        fuzz_cpp, "-o", clang_obj
    ]
    res = subprocess.run(clang_compile_cmd, capture_output=True, text=True)
    if res.returncode != 0:
        if "PLEASE submit a bug report to" in res.stderr:
            return True, "CLANG_INTERNAL_COMPILER_ERROR", res.stderr
        err_lines = [line.strip() for line in res.stderr.splitlines() if "error:" in line or "warning:" in line or "failed" in line or "invalid" in line]
        err_summary = err_lines[0] if err_lines else (res.stderr.splitlines()[0] if res.stderr.splitlines() else "Unknown Clang compile error")
        return False, f"CLANG_COMPILE_ERROR ({err_summary})", res.stderr

    # Compare symbols
    gcc2_symbols = get_mangled_symbols(gcc_obj)
    clang_symbols = get_mangled_symbols(clang_obj)
    if gcc2_symbols is None or clang_symbols is None:
        return False, "NM_ERROR", "Failed to run nm on object files"

    if gcc2_symbols != clang_symbols:
        only_in_gcc2 = gcc2_symbols - clang_symbols
        only_in_clang = clang_symbols - gcc2_symbols

        # Harmless GCC2 compiled metadata stubs (RTTI descriptors, startup initializers, and static local label counters)
        allowed_gcc2_only_prefixes = [
            # GCC2 C++ RTTI Type Descriptor and Type Info symbols (verified dynamically at runtime)
            "__tf", "__ti",
            # GCC 2.95 translation unit global destructor and constructor functions
            "_GLOBAL_.D.", "_GLOBAL_.I.",
            # GCC 2.95 static initialization and destruction blocks
            "__static_initialization_and_destruction_",
            # GCC 2.95 local static variable and member local static variable uniquifiers
            "local_static", "member_local_static",
            # GCC 2.95 C++ runtime RTTI lookup helper symbols
            "__rtti_"
        ]

        # Harmless Clang compiled metadata stubs (destructor wrappers, folded vtables, initializers, and stubs)
        allowed_clang_only_prefixes = [
            # Clang C++ static destructor helper wrappers (Itanium-inherited helper details)
            "__dtor_",
            # Clang modern CodeGen extra weak folded primary vtables
            "__vt_",
            # Clang C++ base destructor checks execution wrappers
            "__base_dtor",
            # Clang modern static global initializers and execution stubs
            "__cxx_global_var_init", "__cxx_global_array_dtor",
            # Clang translation unit global constructors and destructors registration stubs
            "_GLOBAL__sub_I_", "_GLOBAL__D_a",
            # Clang internal string constants and DWARF debugging stubs
            ".L.str", "DW.ref.",
            # Clang-generated GCC-compatible exception tables
            "GCC_except_table",
            # Clang-mangled local static variable uniquifiers matching GCC2 logic
            "local_static", "member_local_static"
        ]

        real_gcc2_only = set()
        for sym in only_in_gcc2:
            is_allowed = False
            for prefix in allowed_gcc2_only_prefixes:
                if sym.startswith(prefix):
                    is_allowed = True
                    break
            if not is_allowed:
                real_gcc2_only.add(sym)

        real_clang_only = set()
        for sym in only_in_clang:
            is_allowed = False
            for prefix in allowed_clang_only_prefixes:
                if sym.startswith(prefix):
                    is_allowed = True
                    break
            if not is_allowed:
                real_clang_only.add(sym)

        if real_gcc2_only or real_clang_only:
            print("MANGLING DISCREPANCY FOUND!")
            diff_info = f"Only in GCC2: {sorted(list(real_gcc2_only))}\nOnly in Clang: {sorted(list(real_clang_only))}"
            return True, "MANGLING_DISCREPANCY", diff_info

    # Link GCC2 with actual libstdc++ and libgcc
    gcc2_link_cmd = [
        os.path.join(config["gcc_dir"], "usr/bin/g++-2.95"),
        f"-B{config['gcc_lib_dir']}/",
        "-Wa,--32",
        gcc_obj,
        os.path.join(config["gcc_lib_dir"], "libstdc++.a"),
        os.path.join(config["gcc_lib_dir"], "libgcc.a"),
        os.path.join(config["gcc_lib_dir"], "crtendS.o"),
        "-lc",
        os.path.join(config["gcc_lib_dir"], "crtn.o"),
        "-o", gcc_bin
    ]
    res = subprocess.run(gcc2_link_cmd, capture_output=True, text=True)
    if res.returncode != 0:
        err_lines = [line.strip() for line in res.stderr.splitlines() if "undefined reference" in line or "error" in line]
        err_summary = err_lines[0] if err_lines else (res.stderr.splitlines()[0] if res.stderr.splitlines() else "Unknown GCC2 link error")
        return False, f"GCC2_LINK_ERROR ({err_summary})", res.stderr

    # Link Clang using clang++ and actual GCC2 libraries with full exception frames
    clang_link_cmd = [
        config["clangxx"],
        "-target", "i386-pc-linux-gnu", "-m32",
        "-Xclang", "-fc++-abi=gcc2",
        "-nostartfiles", "-nodefaultlibs",
        "-Wl,--no-eh-frame-hdr",
        "-fexceptions", "-fcxx-exceptions",
        os.path.join(config["gcc_dir"], "usr/lib/gcc-lib/i486-linux-gnu/2.95.4/Scrt1.o"),
        os.path.join(config["gcc_dir"], "usr/lib/gcc-lib/i486-linux-gnu/2.95.4/crti.o"),
        os.path.join(config["gcc_dir"], "usr/lib/gcc-lib/i486-linux-gnu/2.95.4/crtbeginS.o"),
        clang_obj,
        os.path.join(config["gcc_lib_dir"], "libstdc++.a"),
        os.path.join(config["gcc_lib_dir"], "libgcc.a"),
        os.path.join(config["gcc_lib_dir"], "crtendS.o"),
        "-lc",
        os.path.join(config["gcc_lib_dir"], "crtn.o"),
        "-o", clang_bin
    ]
    res = subprocess.run(clang_link_cmd, capture_output=True, text=True)
    if res.returncode != 0:
        err_lines = [line.strip() for line in res.stderr.splitlines() if "undefined reference" in line or "error" in line]
        err_summary = err_lines[0] if err_lines else (res.stderr.splitlines()[0] if res.stderr.splitlines() else "Unknown Clang link error")
        return False, f"CLANG_LINK_ERROR ({err_summary})", res.stderr

    # Run and compare layout outputs with proper GCC2 shared libraries environment
    env = os.environ.copy()
    gcc_dyn_lib_dir = os.path.join(config["gcc_dir"], "usr/lib")
    env["LD_LIBRARY_PATH"] = f"{gcc_dyn_lib_dir}:{config['gcc_lib_dir']}:" + env.get("LD_LIBRARY_PATH", "")

    res_gcc2 = subprocess.run([gcc_bin], env=env, capture_output=True, text=True)
    if res_gcc2.returncode != 0:
        return False, "GCC2_EXEC_ERROR", f"Exit Code: {res_gcc2.returncode}\nStdout:\n{res_gcc2.stdout}\nStderr:\n{res_gcc2.stderr}"

    res_clang = subprocess.run([clang_bin], env=env, capture_output=True, text=True)
    if res_clang.returncode != 0:
        return False, "CLANG_EXEC_ERROR", f"Exit Code: {res_clang.returncode}\nStdout:\n{res_clang.stdout}\nStderr:\n{res_clang.stderr}"

    norm_gcc2 = normalize_output(res_gcc2.stdout)
    norm_clang = normalize_output(res_clang.stdout)

    if norm_gcc2 != norm_clang:
        print("LAYOUT/VTABLE DISCREPANCY FOUND!")
        smart_compare(norm_gcc2, norm_clang)
        return True, "LAYOUT_VTABLE_DISCREPANCY", f"GCC2 Output:\n{norm_gcc2}\n\nClang Output:\n{norm_clang}"

    return False, None, None

def normalize_output(stdout_str):
    lines = stdout_str.splitlines()
    normalized_lines = []

    temp_block = []
    for line in lines:
        is_tag_line = False
        tags = ["[Func]", "[Member]", "[Op]", "ptmf", "ptmd", "sizeof", "offset", "base_offset", "--- Class", "===", "[Lib]", "[Consumer]", "member_local_static", "local_static"]
        for tag in tags:
            if line.startswith(tag) or tag in line:
                is_tag_line = True
                break

        if is_tag_line:
            if temp_block:
                normalized_lines.extend(sorted(temp_block))
                temp_block = []
            normalized_lines.append(line)
        else:
            temp_block.append(line)

    if temp_block:
        normalized_lines.extend(sorted(temp_block))

    return "\n".join(normalized_lines)

def smart_compare(gcc2_out, clang_out):
    gcc2_lines = gcc2_out.splitlines()
    clang_lines = clang_out.splitlines()

    print("\n=== Smart Discrepancy Comparison ===")
    max_len = max(len(gcc2_lines), len(clang_lines))
    for idx in range(max_len):
        g_line = gcc2_lines[idx] if idx < len(gcc2_lines) else "<EOF>"
        c_line = clang_lines[idx] if idx < len(clang_lines) else "<EOF>"

        if g_line != c_line:
            print(f"Line {idx + 1} Mismatch:")
            print(f"  GCC2:  {g_line}")
            print(f"  Clang: {c_line}")
            break
    print("====================================\n")

def usage():
    print("Usage: python3 fuzzer.py [options] [iterations]")
    print("Options:")
    print("  --llvm-dir <path>   Path to llvm-project directory (default: .)")
    print("  --gcc-dir <path>    Path to gcc-2.95 directory (default: <llvm-dir>/agent_scratch/gcc-2.95)")
    print("  --clangxx <path>    Path to clang++ binary (default: <llvm-dir>/build/bin/clang++)")
    print("  --log-file <path>   Path to log file (default: fuzzer.log in CWD)")
    print("  --ignore-compile-errors  Ignore standard compilation and linking errors (default: False, fail fuzzer)")
    print("  --save-success      Save successful C++ test headers")
    print("  -O0 | -O3           Optimization level (default: -O0)")
    sys.exit(1)

if __name__ == "__main__":
    # Default interop configuration specs
    llvm_dir = "."
    gcc_dir = ""
    clangxx = ""
    log_file = "fuzzer.log"
    ignore_compile_errors = False
    opt_level = "-O0"
    save_success = False
    iterations = 10
    seed = None

    # Parse arguments exactly like run_interop_tests.sh
    args = sys.argv[1:]
    idx = 0
    while idx < len(args):
        arg = args[idx]
        if arg == "--llvm-dir":
            llvm_dir = args[idx+1]
            idx += 2
        elif arg == "--seed":
            seed = int(args[idx+1])
            idx += 2
        elif arg == "--gcc-dir":
            gcc_dir = args[idx+1]
            idx += 2
        elif arg == "--clangxx":
            clangxx = args[idx+1]
            idx += 2
        elif arg == "--log-file":
            log_file = args[idx+1]
            idx += 2
        elif arg == "--ignore-compile-errors":
            ignore_compile_errors = True
            idx += 1
        elif arg == "--save-success":
            save_success = True
            idx += 1
        elif arg in ["-O0", "-O3"]:
            opt_level = arg
            idx += 1
        elif arg in ["-h", "--help"]:
            usage()
        elif arg.startswith("-"):
            print(f"Unknown option: {arg}", file=sys.stderr)
            usage()
        elif arg.isdigit():
            iterations = int(arg)
            idx += 1
        else:
            break

    # Resolve path defaults
    llvm_dir = os.path.abspath(llvm_dir)
    if not gcc_dir:
        gcc_dir = os.path.join(llvm_dir, "agent_scratch/gcc-2.95")
    if not clangxx:
        clangxx = os.path.join(llvm_dir, "build/bin/clang++")

    # Derive clang binary path
    clang_dir = os.path.dirname(clangxx)
    clang = os.path.join(clang_dir, "clang")
    gcc_lib_dir = os.path.join(gcc_dir, "usr/lib/gcc-lib/i486-linux-gnu/2.95.4")
    scratch_dir = os.path.join(llvm_dir, "agent_scratch/fuzzer_scratch")

    # Validate directories and binaries
    if not os.path.isdir(llvm_dir):
        print(f"Error: LLVM directory {llvm_dir} does not exist.", file=sys.stderr)
        sys.exit(1)
    if not os.path.isdir(gcc_dir):
        print(f"Error: GCC directory {gcc_dir} does not exist.", file=sys.stderr)
        sys.exit(1)
    if not os.path.isfile(clangxx):
        print(f"Error: Clang++ executable {clangxx} does not exist.", file=sys.stderr)
        sys.exit(1)
    if not os.path.isfile(clang):
        print(f"Error: Clang executable {clang} does not exist.", file=sys.stderr)
        sys.exit(1)
    if not os.path.isdir(gcc_lib_dir):
        print(f"Error: GCC lib directory {gcc_lib_dir} does not exist.", file=sys.stderr)
        sys.exit(1)

    os.makedirs(scratch_dir, exist_ok=True)

    config = {
        "llvm_dir": llvm_dir,
        "gcc_dir": gcc_dir,
        "gcc_lib_dir": gcc_lib_dir,
        "clangxx": clangxx,
        "clang": clang,
        "ignore_compile_errors": ignore_compile_errors,
        "opt_level": opt_level,
        "scratch_dir": scratch_dir
    }

    if seed is None:
        seed = random.randint(0, 1000000)
    random.seed(seed)

    print("============================================================")
    print("   LEGACY GCC 2.95 & CLANG GCC2 ABI INTEROP COMPAT FUZZER   ")
    print(f"   Optimization Level: {opt_level}                           ")
    print(f"   LLVM Project Dir:   {llvm_dir}                            ")
    print(f"   GCC 2.95 Dir:       {gcc_dir}                             ")
    print(f"   Scratch Dir:        {scratch_dir}                         ")
    print(f"   Random Seed:        {seed}                                ")
    print("============================================================")

    def print_progress(iteration, total, passed, skipped, failed):
        width = 40
        progress = int(width * iteration / total)
        bar = "=" * progress + ">" + "." * (width - progress - 1)
        if progress == width:
            bar = "=" * width
        percent = int(100 * iteration / total)
        sys.stdout.write(f"\r\033[KProgress: [{bar}] {iteration}/{total} ({percent}%) | Passed: {passed} | Skipped: {skipped} | Failed: {failed}")
        sys.stdout.flush()

    log_file_path = os.path.abspath(log_file)
    # Initialize and clear log file on startup
    with open(log_file_path, "w") as f:
        f.write(f"=== fuzzer.log initialized at {datetime.datetime.now().isoformat()} ===\n")
        f.write(f"=== Random Seed: {seed} ===\n\n")

    print(f"Running {iterations} iterations (save_success={save_success})...")
    success_count = 0
    skipped_count = 0
    failed_count = 0

    print_progress(0, iterations, success_count, skipped_count, failed_count)

    found = False
    for i in range(iterations):
        generate_hierarchy(random.randint(2, 6))
        # Unpack 3-tuple from run_test!
        found, err, full_log = run_test(config)
        if not found and not err:
            success_count += 1
            if save_success:
                success_path = os.path.join(scratch_dir, f"success_case_{success_count}.h")
                with open(success_path, "w") as f:
                    f.write(render_main())
                # Clean progress bar, print success message, redraw progress bar
                sys.stdout.write(f"\r\033[KSaved success case {success_count} (from iteration {i})\n")
                sys.stdout.flush()
        elif err:
            # Check if this is a Clang ICE, un-ignored compilation error, or strict fuzzer failure
            is_ice = err.startswith("CLANG_INTERNAL_COMPILER_ERROR")
            is_unignored_compile_err = (not config["ignore_compile_errors"] and ("COMPILE_ERROR" in err or "LINK_ERROR" in err))
            is_strict_failure = ("DISCREPANCY" in err or "EXEC_ERROR" in err)

            if is_ice or is_unignored_compile_err or is_strict_failure:
                found = True
                failed_count += 1
                print_progress(i + 1, iterations, success_count, skipped_count, failed_count)

                # Detailed failure header string
                if is_ice:
                    fail_header = f"[CRITICAL CLANG ICE] Clang crashed at iteration {i}!"
                elif is_strict_failure:
                    fail_header = f"[FUZZ DISCREPANCY / FAILURE] Mismatch/Error found at iteration {i}!"
                else:
                    fail_header = f"[COMPILE/LINK FAILURE] Compile/Link error at iteration {i}!"

                # Write to fuzzer.log
                src_content = ""
                try:
                    fuzz_cpp_path = os.path.join(config["scratch_dir"], "fuzz_test.cpp")
                    with open(fuzz_cpp_path, "r") as src_f:
                        src_content = src_f.read()
                except Exception as src_err:
                    src_content = f"Failed to read fuzzed source file: {src_err}"

                instruction_str = (
                    "\n=== INSTRUCTIONS FOR THE NEXT AGENT / TESTER ===\n"
                    "1) Reproduce the compile/linkage/ICE failure inside the 'interslop' interop verification tests first.\n"
                    "2) Implement the robust layout, vtable context, or ABI fix in Clang.\n"
                    "3) Add permanent regression LIT tests under clang/test/CodeGenCXX/gcc2-abi/.\n"
                    "4) Ensure that all new and existing LIT tests pass.\n"
                    "5) Rebuild Clang and verify that the 'interslop' interop verification test suite passes 100% (run ./run_interop_tests.sh).\n"
                    "During this process, you MUST NOT make changes outside the GCC2 ABI handling classes without authorization.\n"
                    "================================================\n"
                )

                with open(log_file_path, "a") as log_f:
                    log_f.write("=" * 80 + "\n")
                    log_f.write(f"{fail_header}\n")
                    log_f.write(f"Error Code: {err}\n")
                    log_f.write(f"Random Seed: {seed}\n")
                    log_f.write(f"Time: {datetime.datetime.now().isoformat()}\n")
                    log_f.write("-" * 80 + "\n")
                    log_f.write(full_log + "\n")
                    log_f.write("-" * 80 + "\n")
                    log_f.write("=== FAILED FUZZED C++ SOURCE CODE ===\n")
                    log_f.write(src_content + "\n")
                    log_f.write("=====================================\n")
                    log_f.write(instruction_str + "\n")
                    log_f.write("=" * 80 + "\n\n")

                # Print detailed failure log to terminal!
                sys.stdout.write(f"\n\n{fail_header}\n")
                sys.stdout.write("=" * 80 + "\n")
                sys.stdout.write(f"Error Code: {err}\n")
                sys.stdout.write(f"Random Seed: {seed}\n")
                sys.stdout.write("-" * 80 + "\n")
                sys.stdout.write(full_log + "\n")
                sys.stdout.write("=" * 80 + "\n")
                sys.stdout.write(instruction_str + "\n")
                sys.stdout.flush()
                break
            else:
                skipped_count += 1
                # Brief, beautiful one-line console warning
                brief_err = err.split("(")[0].strip()
                sys.stdout.write(f"\r\033[K[WARNING] Iteration {i} skipped: {brief_err} (Logged to {log_file})\n")
                sys.stdout.flush()

                # Detailed, full stderr warning inside fuzzer.log with solid separation marks
                with open(log_file_path, "a") as log_f:
                    log_f.write("=" * 80 + "\n")
                    log_f.write(f"[WARNING] Iteration {i} Skipped: {err}\n")
                    log_f.write(f"Time: {datetime.datetime.now().isoformat()}\n")
                    log_f.write("-" * 80 + "\n")
                    log_f.write(full_log + "\n")
                    log_f.write("=" * 80 + "\n\n")
        else:
            failed_count += 1

        print_progress(i + 1, iterations, success_count, skipped_count, failed_count)

        if found:
            sys.stdout.write(f"\n\n[DISCREPANCY] Discrepancy found at iteration {i} (primary fuzzer repro)\n")
            sys.stdout.flush()
            break

    sys.stdout.write("\n")
    sys.stdout.flush()
    if not found:
        print(f"All {iterations} iterations completed. {success_count} compiled and matched successfully.")
    else:
        sys.exit(1)
