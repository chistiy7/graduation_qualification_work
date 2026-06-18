from __future__ import annotations

import argparse
import sys
from pathlib import Path

from .audio_merge import merge_audio_tracks
from .config import PROMPTS_DIR, Settings
from .log import progress
from .pipeline import process_audio
from .fix_scan import render_fix_queue_md, scan_fix_markers


def _load_dotenv() -> None:
    env_path = Path(__file__).resolve().parents[1] / ".env"
    if not env_path.is_file():
        return
    for line in env_path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, _, value = line.partition("=")
        key = key.strip()
        value = value.strip().strip('"').strip("'")
        if key and key not in __import__("os").environ:
            __import__("os").environ[key] = value


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="context_service",
        description=(
            "Транскрибация аудио разговоров и извлечение контекста для диплома "
            "(испытательная лаборатория, маршруты испытаний, ПО)."
        ),
    )
    sub = parser.add_subparsers(dest="command", required=True)

    proc = sub.add_parser("process", help="Обработать один или несколько аудиофайлов")
    proc.add_argument(
        "audio",
        type=Path,
        nargs="+",
        help="Путь к аудио (.mp3, .m4a, .webm, …). Несколько файлов — только с --merge",
    )
    proc.add_argument(
        "--merge",
        action="store_true",
        help="Сначала склеить все указанные дорожки в одну, затем транскрибировать",
    )
    proc.add_argument(
        "--title",
        "-t",
        required=True,
        help="Название сессии (например: «Консультация с научруком 2025-05-27»)",
    )
    proc.add_argument(
        "--notes",
        "-n",
        default="",
        help="Дополнительные заметки (сохраняются в notes.md сессии)",
    )
    proc.add_argument(
        "--language",
        default="ru",
        help="Язык для Whisper (ru, en, …) или пусто для авто",
    )
    proc.add_argument(
        "--output-dir",
        type=Path,
        default=None,
        help="Каталог сессий (по умолчанию diploma/03_capture/sessions)",
    )
    proc.add_argument(
        "--model",
        "-m",
        default=None,
        help="Модель Whisper: tiny, base, small, medium (быстрее: tiny)",
    )
    proc.add_argument(
        "--fix",
        action="store_true",
        help="После обработки сессии обновить очередь правок по меткам #todo/#",
    )

    sub.add_parser("info", help="Показать пути и переменные окружения по умолчанию")

    clean = sub.add_parser(
        "clean",
        help="Удалить пустые каталоги сессий (после прерванных запусков)",
    )
    clean.add_argument(
        "--output-dir",
        type=Path,
        default=None,
        help="Каталог sessions (по умолчанию diploma/03_capture/sessions)",
    )

    fix = sub.add_parser("fix", help="Собрать очередь правок по меткам #todo/#")
    fix.add_argument(
        "--root",
        type=Path,
        default=None,
        help="Корень сканирования (по умолчанию корень diploma)",
    )
    fix.add_argument(
        "--out",
        type=Path,
        default=None,
        help="Куда сохранить отчёт (по умолчанию 00_context/fix_queue.md)",
    )
    return parser


def cmd_clean(settings: Settings, output_dir: Path | None) -> int:
    root = output_dir or settings.output_dir
    if not root.is_dir():
        print(f"Каталог не найден: {root}")
        return 1
    removed = 0
    for path in sorted(root.iterdir()):
        if not path.is_dir():
            continue
        if any(path.iterdir()):
            continue
        path.rmdir()
        print(f"Удалено: {path.name}")
        removed += 1
    print(f"Готово: удалено {removed} пустых каталогов")
    return 0


def cmd_info(settings: Settings) -> int:
    print("Context service — захват контекста для диплома")
    print(f"  Выходные сессии: {settings.output_dir}")
    print(f"  Контекст диплома: {settings.context_dir}")
    print(f"  Whisper: model={settings.whisper_model}, device={settings.whisper_device}")
    print(f"  Брифинг: в чате Cursor по {PROMPTS_DIR / 'extract_context.md'}")
    return 0


def cmd_process(args: argparse.Namespace, settings: Settings) -> int:
    if args.output_dir or args.model:
        settings = Settings(
            whisper_model=args.model or settings.whisper_model,
            whisper_device=settings.whisper_device,
            whisper_compute_type=settings.whisper_compute_type,
            output_dir=args.output_dir or settings.output_dir,
            context_dir=settings.context_dir,
        )

    audio_paths = [p.expanduser().resolve() for p in args.audio]
    for p in audio_paths:
        if not p.is_file():
            print(f"Ошибка: аудиофайл не найден: {p}", file=sys.stderr)
            return 1

    if len(audio_paths) > 1 and not args.merge:
        print(
            "Ошибка: указано несколько файлов — добавьте --merge для склейки.",
            file=sys.stderr,
        )
        return 1
    if args.merge and len(audio_paths) < 2:
        print("Ошибка: --merge требует минимум 2 файла.", file=sys.stderr)
        return 1
    merged_temp: Path | None = None
    if args.merge:
        try:
            merged_temp = merge_audio_tracks(audio_paths)
            audio = merged_temp
        except Exception as exc:
            print(f"Ошибка склейки: {exc}", file=sys.stderr)
            return 1
    else:
        audio = audio_paths[0]

    language = args.language.strip() or None
    progress(f"Файл: {audio} ({audio.stat().st_size / (1024 * 1024):.1f} МБ)")
    result = None
    try:
        result = process_audio(
            audio,
            title=args.title,
            settings=settings,
            notes=args.notes,
            language=language,
        )
    except Exception as exc:
        print(f"Ошибка: {exc}", file=sys.stderr)
        return 1
    finally:
        if merged_temp and merged_temp.is_file():
            merged_temp.unlink(missing_ok=True)

    if result is None:
        return 1

    print(f"Готово: {result.session_dir}")
    print(f"  → {result.transcript_path.name}")
    print(
        "  → брифинг: в чате — «сделай брифинг по transcript.md» "
        f"(промпт: {PROMPTS_DIR / 'extract_context.md'})"
    )

    if args.fix:
        try:
            out = _run_fix(settings, root=None, out=None)
            print(f"  → fix: {out}")
        except Exception as exc:
            print(f"Предупреждение: не удалось обновить fix-очередь: {exc}", file=sys.stderr)
    return 0


def _run_fix(settings: Settings, *, root: Path | None, out: Path | None) -> Path:
    scan_root = (root or Path(__file__).resolve().parents[1]).expanduser().resolve()
    out_path = (
        (out or (settings.context_dir / "fix_queue.md"))
        .expanduser()
        .resolve()
    )
    items = scan_fix_markers(scan_root)
    md = render_fix_queue_md(items, root=scan_root)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(md, encoding="utf-8")
    return out_path


def cmd_fix(args: argparse.Namespace, settings: Settings) -> int:
    try:
        out_path = _run_fix(settings, root=args.root, out=args.out)
    except Exception as exc:
        print(f"Ошибка: {exc}", file=sys.stderr)
        return 1
    print("Готово:")
    print(f"  → {out_path}")
    return 0


def main(argv: list[str] | None = None) -> int:
    _load_dotenv()
    parser = build_parser()
    args = parser.parse_args(argv)
    settings = Settings.from_env()

    if args.command == "info":
        return cmd_info(settings)
    if args.command == "clean":
        return cmd_clean(settings, args.output_dir)
    if args.command == "process":
        return cmd_process(args, settings)
    if args.command == "fix":
        return cmd_fix(args, settings)
    parser.error(f"Неизвестная команда: {args.command}")
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
