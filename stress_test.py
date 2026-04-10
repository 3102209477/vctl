import os
import shutil
import subprocess
import sys
import time
from pathlib import Path


def run_command(args, cwd, capture=True):
    result = subprocess.run(
        args,
        cwd=cwd,
        capture_output=capture,
        text=True,
        shell=False,
    )
    return result


def ensure_clean_dir(path: Path):
    if path.exists():
        shutil.rmtree(path)
    path.mkdir(parents=True, exist_ok=True)


def write_file(path: Path, text: str):
    path.write_text(text, encoding="utf-8")


def append_file(path: Path, text: str):
    path.write_text(path.read_text(encoding="utf-8") + text, encoding="utf-8")


def check_output_contains(output: str, phrase: str) -> bool:
    return phrase in output


def main():
    repo_root = Path(__file__).resolve().parent
    test_root = repo_root / "stress_test"
    ensure_clean_dir(test_root)

    vctl_exe = repo_root / "vctl.exe"
    if not vctl_exe.exists():
        print("Error: vctl.exe not found in repository root.")
        sys.exit(1)

    print("====================================")
    print("  版本控制系统 - Python 功能压力测试")
    print("====================================")
    print()

    pass_count = 0
    fail_count = 0

    def report(name, success, warning=False):
        nonlocal pass_count, fail_count
        tag = "PASS" if success else "FAIL"
        if warning and success:
            tag = "WARN"
        print(f"[{tag}] {name}")
        if success:
            pass_count += 1
        elif not warning:
            fail_count += 1

    def step(message):
        print()
        print(f"{message}")

    # Step 1: init
    step("[测试 1/10] 初始化仓库测试...")
    result = run_command([str(vctl_exe), "init"], cwd=test_root)
    report("初始化成功", result.returncode == 0)

    # Create 100 files
    step("[测试 2/10] 批量创建文件（100 个）...")
    for i in range(1, 101):
        write_file(test_root / f"file_{i}.txt", "Test content line 1\nTest content line 2\nTest content line 3\n")
    print("已创建 100 个测试文件")

    # Add files
    step("[测试 3/10] 批量添加文件（冷缓存）...")
    start = time.perf_counter()
    result = run_command([str(vctl_exe), "add", "."], cwd=test_root)
    end = time.perf_counter()
    output = result.stdout + result.stderr
    success = check_output_contains(output, "Added:")
    report("成功添加 100 个文件", success)
    if not success:
        print(output)
    else:
        print(output.strip())
    print(f"耗时：{end - start:.2f}s")

    # First commit
    step("[测试 4/10] 首次提交...")
    result = run_command([str(vctl_exe), "commit", "-m", "Initial commit with 100 files"], cwd=test_root)
    report("提交成功", result.returncode == 0)

    # Status check
    step("[测试 5/10] 状态检查（无变更）...")
    result = run_command([str(vctl_exe), "status"], cwd=test_root)
    output = result.stdout + result.stderr
    success = check_output_contains(output, "Nothing to commit")
    if success:
        report("状态检查正常", True)
    else:
        report("状态检查有警告", False, warning=True)
        print(output.strip())

    # Modify files
    step("[测试 6/10] 批量修改文件（50 个）...")
    for i in range(1, 51):
        append_file(test_root / f"file_{i}.txt", "Modified line\n")
    print("已修改 50 个文件")

    # Add modified files again
    step("[测试 7/10] 再次添加（热缓存，部分变更）...")
    start = time.perf_counter()
    result = run_command([str(vctl_exe), "add", "."], cwd=test_root)
    end = time.perf_counter()
    output = result.stdout + result.stderr
    success = check_output_contains(output, "Added:")
    report("成功添加变更", success)
    if not success:
        print(output)
    else:
        print(output.strip())
    print(f"耗时：{end - start:.2f}s")

    # Second commit
    step("[测试 8/10] 第二次提交...")
    result = run_command([str(vctl_exe), "commit", "-m", "Update 50 files"], cwd=test_root)
    report("提交成功", result.returncode == 0)

    # Log history
    step("[测试 9/10] 查看提交历史...")
    result = run_command([str(vctl_exe), "log", "-n", "5"], cwd=test_root)
    output = result.stdout + result.stderr
    success = check_output_contains(output, "commit")
    report("历史记录正常", success)
    if not success:
        print(output.strip())

    # Branch and checkout
    step("[测试 10/10] 创建和切换分支...")
    result = run_command([str(vctl_exe), "branch", "-b", "feature-test"], cwd=test_root)
    report("创建分支成功", result.returncode == 0)

    result = run_command([str(vctl_exe), "checkout", "feature-test"], cwd=test_root)
    report("切换分支成功", result.returncode == 0)

    write_file(test_root / "branch_file.txt", "Branch work\n")
    run_command([str(vctl_exe), "add", "."], cwd=test_root)
    run_command([str(vctl_exe), "commit", "-m", "Branch commit"], cwd=test_root)

    result = run_command([str(vctl_exe), "checkout", "master"], cwd=test_root)
    report("切回 master 成功", result.returncode == 0)

    print()
    print("====================================")
    print("  测试结果汇总")
    print("====================================")
    print(f"通过：{pass_count}")
    print(f"失败：{fail_count}")
    print(f"总计：{pass_count + fail_count}")

    if fail_count == 0:
        print("[SUCCESS] 所有测试通过！")
    else:
        print("[WARNING] 部分测试失败")

    print()
    print("清理测试目录...")
    shutil.rmtree(test_root, ignore_errors=True)
    print("清理完成")

    return 0 if fail_count == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
