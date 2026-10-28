async function main() {
    const CWD = window.workingDir;

    return {
        mainText: "OpenSupaplex",
        secondaryText: "Open source reimplementation of Supaplex",
        onclick: async () => {
            return {
                path: CWD + "/eboot.elf",
                cwd: CWD,
                env: {HOME: CWD},
                args: []
            };
        }
    };
}
