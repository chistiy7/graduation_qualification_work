from __future__ import annotations

import re


def build_local_brief_markdown(*, title: str, transcript_text: str) -> str:
    """
    Формирует `context.md` в единой структуре `prompts/extract_context.md`.
    """
    text = _normalize(transcript_text)
    sentences = _split_sentences(text)

    summary = _pick(sentences, patterns=[r"\bглава\b", r"\bмодель\b", r"\bметрик", r"\bсетк", r"\bячейк"], limit=6)
    domain = _pick(sentences, patterns=[r"\bлаборатор", r"\bстенд", r"\bиспытан", r"\bобразц", r"\bогранич", r"\bзапрет"], limit=18)
    reqs = _pick(sentences, patterns=[r"\bнужно\b", r"\bнадо\b", r"\bдолжн", r"\bтребу", r"\bследует\b"], limit=18)
    model = _pick(sentences, patterns=[r"\bK\b", r"\bT\b", r"\bC\b", r"\bN\b", r"\bη\b", r"\bвесов", r"\bнормир", r"\bцеле", r"\bкритер", r"\bкоэффиц"], limit=18)

    quotes = _pick(
        sentences,
        patterns=[
            r"\d",  # любые числа
            r"\b1×1\b|\b2×2\b|\b3×3\b",
            r"\bмы договор", r"\bне (?:испытываем|учитываем)\b",
        ],
        limit=12,
    )

    glossary_terms = _extract_glossary_candidates(sentences, limit=10)

    def md_list(items: list[str]) -> str:
        if not items:
            return "нет данных"
        return "\n".join(f"- {it}" for it in items)

    def md_table(rows: list[tuple[str, str]]) -> str:
        if not rows:
            return "нет данных"
        lines = ["| Термин | Определение / контекст |", "|---|---|"]
        lines.extend([f"| {t} | {d} |" for t, d in rows])
        return "\n".join(lines)

    return "\n".join(
        [
            "## Краткое резюме",
            _summary_text(title=title, items=summary),
            "",
            "## Предметная область",
            md_list(domain),
            "",
            "## Требования к программе",
            md_list(reqs),
            "",
            "## Модель и метрики",
            md_list(model),
            "",
            "## Термины для глоссария",
            md_table([(t, _define_term(t)) for t in glossary_terms]),
            "",
            "## Решения и договорённости",
            md_list(_pick(sentences, patterns=[r"\bдоговор", r"\bпринято\b", r"\bфиксир", r"\bне учитыв"], limit=12)),
            "",
            "## Открытые вопросы",
            md_list(_pick(sentences, patterns=[r"\bвопрос\b", r"\bуточн", r"\bнепонят", r"\bнужно найти\b"], limit=12)),
            "",
            "## Цитаты и цифры",
            md_list(quotes),
            "",
            "## Связь с главами диплома",
            md_list(_chapter_links(sentences)),
            "",
        ]
    )


def _normalize(text: str) -> str:
    text = text.replace("\u00a0", " ")
    text = re.sub(r"\s+", " ", text).strip()
    return text


def _split_sentences(text: str) -> list[str]:
    if not text:
        return []
    parts = re.split(r"(?<=[.!?])\s+|(?<=\n)\s+|(?<=\.)\s+", text)
    out = [p.strip(" -\t") for p in parts if p and p.strip(" -\t")]
    return out


def _pick(sentences: list[str], *, patterns: list[str], limit: int) -> list[str]:
    if not sentences:
        return []
    rx = re.compile("|".join(f"(?:{p})" for p in patterns), flags=re.IGNORECASE)
    chosen: list[str] = []
    seen = set()
    for s in sentences:
        if rx.search(s) and s not in seen:
            chosen.append(s)
            seen.add(s)
        if len(chosen) >= limit:
            break
    return chosen


def _extract_glossary_candidates(sentences: list[str], limit: int) -> list[str]:
    # вытаскиваем “терминообразные” слова/словосочетания из частых тем
    candidates: list[str] = []
    rx = re.compile(
        r"\b(образец|партия образцов|испытани[ея]|испытательн(?:ый|ая)\s+стенд|перенастройк[аи]|"
        r"себестоимост[ьи]|метрик[аеи]|ячейк[аи]|сетк[аи]|запретн(?:ая|ые)\s+зон[аы])\b",
        flags=re.IGNORECASE,
    )
    for s in sentences:
        for m in rx.finditer(s):
            term = m.group(0).lower()
            term = term.replace("испытания", "испытание").replace("испытаниями", "испытание")
            if term not in candidates:
                candidates.append(term)
            if len(candidates) >= limit:
                return candidates
    return candidates


def _summary_text(*, title: str, items: list[str]) -> str:
    if not items:
        return "нет данных"
    compact = " ".join(items[:4])
    return (
        f"Разговор «{title}» связан с дипломом по планированию и оптимизации "
        f"испытательной лаборатории. В транскрипте выделены сведения, относящиеся "
        f"к требованиям, предметной области, модели, метрикам и открытым вопросам. "
        f"Ключевые фрагменты: {compact}"
    )


def _define_term(term: str) -> str:
    normalized = term.lower()
    definitions = {
        "образец": "Объект, для которого задается маршрут испытаний.",
        "партия образцов": "Группа образцов, проходящих заданные сценарии испытаний.",
        "испытание": "Проверка или воздействие над образцом в рамках маршрута испытаний.",
        "испытательный стенд": "Оборудование лаборатории, используемое для выполнения испытаний.",
        "перенастройка": "Изменение состояния стенда или маршрута между испытательными операциями.",
        "себестоимость": "Затраты на выполнение испытаний с учетом операций и ограничений.",
        "метрика": "Показатель, используемый для оценки маршрута или размещения.",
        "ячейка": "Элемент сетки лаборатории, где может размещаться стенд или часть маршрута.",
        "сетка": "Дискретное представление пространства испытательной лаборатории.",
        "запретная зона": "Область, недоступная для размещения стендов или прохождения маршрута.",
    }
    return definitions.get(normalized, "Термин упомянут в транскрипте; контекст требует проверки.")


def _chapter_links(sentences: list[str]) -> list[str]:
    links: list[str] = []
    if _pick(sentences, patterns=[r"\bлаборатор", r"\bстенд", r"\bиспытан", r"\bобразц"], limit=1):
        links.append("Глава 1: использовать факты о предметной области, образцах, испытаниях и лаборатории.")
    if _pick(sentences, patterns=[r"\bмодель\b", r"\bсетк", r"\bячейк", r"\bмаршрут"], limit=1):
        links.append("Глава 2: описать модель сетки, ячеек, маршрутов и входных параметров.")
    if _pick(sentences, patterns=[r"\bцелев", r"\bкритер", r"\bметрик", r"\bсебестоим"], limit=1):
        links.append("Глава 3: вынести метрики, критерии оптимизации и себестоимость испытаний.")
    if _pick(sentences, patterns=[r"\bпрограмм", r"\bвывод", r"\bвход", r"\bсценари"], limit=1):
        links.append("Глава 4: описать реализацию ПО, сценарии, входные данные и выходные отчеты.")
    if _pick(sentences, patterns=[r"\bтест", r"\bпример", r"\bвариант", r"\bрезультат"], limit=1):
        links.append("Глава 5: использовать примеры, тестовые сценарии и результаты расчетов.")
    return links

